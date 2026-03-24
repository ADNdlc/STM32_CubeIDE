#include "sdmmc_storage_drv.h"
#include "MemPool.h"
#include <string.h>

// 引入 STM32 HAL 库头文件
#include "main.h"

#define SD_BLOCK_SIZE 512
#define SD_TIMEOUT 1000 // 超时时间(ms)

/* ================== 接口函数实现 ================== */

static int sdmmc_drv_init(storage_device_t *self) {
  sdmmc_storage_device_t *drv = (sdmmc_storage_device_t *)self;
  HAL_SD_CardInfoTypeDef card_info;

  // ==========================================================
  // 硬件与软件的双重复位
  // ==========================================================
  // 1. 硬件级复位：清理拔卡导致的内部数据状态机(DPSM)死锁
  if (drv->hsd->Instance == SDMMC1) {
      __HAL_RCC_SDMMC1_FORCE_RESET();
      __HAL_RCC_SDMMC1_RELEASE_RESET();
  }
#if defined(SDMMC2)
  else if (drv->hsd->Instance == SDMMC2) {
      __HAL_RCC_SDMMC2_FORCE_RESET();
      __HAL_RCC_SDMMC2_RELEASE_RESET();
  }
#endif

  // 2. 软件级复位：【解决状态检查次都卡死 5 秒才恢复】
  // 强制把 HAL 状态机改回 RESET！
  // 这样 HAL_SD_Init 就会乖乖去调用 HAL_SD_MspInit，重新打开时钟和引脚。
  // 时钟一旦恢复，硬件的响应超时(CTIMEOUT)就会在几毫秒内触发
  drv->hsd->State = HAL_SD_STATE_RESET;
  // ==========================================================

  // 3. 执行完整的卡初始化和枚举流程
  // 如果此时没有插卡，瞬间就会返回失败
  if (HAL_SD_Init(drv->hsd) != HAL_OK) {
    return -1;
  }

  // 获取卡信息
  if (HAL_SD_GetCardInfo(drv->hsd, &card_info) != HAL_OK) {
    return -2;
  }

  // 填充设备缓存信息
  self->info.type = STORAGE_TYPE_SD_CARD;
  self->info.read_size = SD_BLOCK_SIZE;
  self->info.write_size = SD_BLOCK_SIZE;
  self->info.erase_size = SD_BLOCK_SIZE;
  self->info.total_size = (uint64_t)card_info.BlockNbr * card_info.BlockSize;

  // 获取卡的 CID
  HAL_SD_CardCIDTypeDef card_cid;
  if (HAL_SD_GetCardCID(drv->hsd, &card_cid) == HAL_OK) {
    self->info.device_id[0] = (uint8_t)(card_cid.ManufacturerID);
    self->info.device_id[1] = (uint8_t)(card_cid.OEM_AppliID >> 8);
    self->info.device_id[2] = (uint8_t)(card_cid.OEM_AppliID & 0xFF);
    self->info.device_id[3] = (uint8_t)(card_cid.ProdRev);
  }

  return 0;
}

static int sdmmc_drv_deinit(storage_device_t *self) {
  sdmmc_storage_device_t *drv = (sdmmc_storage_device_t *)self;
  if (HAL_SD_DeInit(drv->hsd) != HAL_OK) {
    return -1;
  }
  return 0;
}

static storage_status_t sdmmc_drv_get_status(storage_device_t *self) {
  sdmmc_storage_device_t *drv = (sdmmc_storage_device_t *)self;

  if (drv->hsd->State == HAL_SD_STATE_RESET) {
    return STORAGE_STATUS_NOT_INIT;
  }

  // 探测卡状态 (发送 CMD13)
  HAL_SD_CardStateTypeDef card_state = HAL_SD_GetCardState(drv->hsd);

  // 1. 明确判定离线的条件：
  // - 返回 ERROR (通常是 CMD13 彻底超时)
  // - 返回 0x0F (即 15, 总线悬空全为高电平被误读)
  // - 返回 0x00 (也是无效状态)
  if (card_state == HAL_SD_CARD_ERROR ||
      card_state == (HAL_SD_CardStateTypeDef)0x0FU ||
      card_state == 0x00U) {

      // 【关键恢复逻辑】：如果卡在读写中途被拔出，HAL 可能会死锁在 BUSY 状态。
      // 此时需要强制 Abort 解锁底层外设，否则后续无法恢复
      if (drv->hsd->State != HAL_SD_STATE_READY) {
          HAL_SD_Abort(drv->hsd);
      }
      return STORAGE_STATUS_OFFLINE;
  }

  // 2. 处理标准状态
  switch (card_state) {
  case HAL_SD_CARD_TRANSFER:
    return STORAGE_STATUS_OK;

  case HAL_SD_CARD_READY:
  case HAL_SD_CARD_IDENTIFICATION:
  case HAL_SD_CARD_STANDBY:
  case HAL_SD_CARD_DISCONNECTED:
    // 卡已响应，但未准备好数据传输 (可能刚上电，或被软件复位)
    return STORAGE_STATUS_NOT_INIT;

  case HAL_SD_CARD_PROGRAMMING:
  case HAL_SD_CARD_RECEIVING:
  case HAL_SD_CARD_SENDING:
    return STORAGE_STATUS_BUSY;

  default:
    return STORAGE_STATUS_OFFLINE;
  }
}

static int sdmmc_drv_read(storage_device_t *self, uint32_t addr, uint8_t *buf, size_t len) {
  sdmmc_storage_device_t *drv = (sdmmc_storage_device_t *)self;

  if ((addr % SD_BLOCK_SIZE) != 0 || (len % SD_BLOCK_SIZE) != 0) {
    return -1;
  }

  uint32_t block_idx = addr / SD_BLOCK_SIZE;
  uint32_t block_cnt = len / SD_BLOCK_SIZE;

  if (HAL_SD_ReadBlocks(drv->hsd, buf, block_idx, block_cnt, SD_TIMEOUT) != HAL_OK) {
    // 拔卡会导致 Read 报错，直接 Abort 退出即可。
    // 内部残留的硬件死锁，将在下次挂载(Init)时被 FORCE_RESET 彻底清理。
    HAL_SD_Abort(drv->hsd);
    return -2;
  }

  return 0;
}

static int sdmmc_drv_write(storage_device_t *self, uint32_t addr,
                           const uint8_t *data, size_t len) {
  sdmmc_storage_device_t *drv = (sdmmc_storage_device_t *)self;

  if ((addr % SD_BLOCK_SIZE) != 0 || (len % SD_BLOCK_SIZE) != 0) return -1;

  uint32_t block_idx = addr / SD_BLOCK_SIZE;
  uint32_t block_cnt = len / SD_BLOCK_SIZE;

  if (HAL_SD_WriteBlocks(drv->hsd, (uint8_t *)data, block_idx, block_cnt, SD_TIMEOUT) != HAL_OK) {
    HAL_SD_Abort(drv->hsd);
    return -2;
  }

  // 【修复】：带安全跳出的等待循环
  uint32_t tickstart = HAL_GetTick();
  while (1) {
    HAL_SD_CardStateTypeDef state = HAL_SD_GetCardState(drv->hsd);

    if (state == HAL_SD_CARD_TRANSFER) {
      break; // 正常完成
    }

    // 如果突然拔卡，捕捉到异常状态，立刻跳出防止死机
    if (state == HAL_SD_CARD_ERROR || state == (HAL_SD_CardStateTypeDef)0x0FU || state == 0x00U) {
      HAL_SD_Abort(drv->hsd);
      return -3;
    }

    // 超时保护
    if ((HAL_GetTick() - tickstart) >= SD_TIMEOUT) {
      HAL_SD_Abort(drv->hsd);
      return -4;
    }
  }

  return 0;
}

static int sdmmc_drv_erase(storage_device_t *self, uint32_t addr, size_t len) {
  sdmmc_storage_device_t *drv = (sdmmc_storage_device_t *)self;

  if ((addr % SD_BLOCK_SIZE) != 0 || (len % SD_BLOCK_SIZE) != 0) {
    return -1;
  }

  uint32_t start_block = addr / SD_BLOCK_SIZE;
  uint32_t end_block = start_block + (len / SD_BLOCK_SIZE) - 1;

  // SD卡通过起始块和结束块来进行擦除
  if (HAL_SD_Erase(drv->hsd, start_block, end_block) != HAL_OK) {
    HAL_SD_Abort(drv->hsd);
    return -2;
  }

  // 【修复】：带安全跳出的等待循环
  uint32_t tickstart = HAL_GetTick();
  while (1) {
    HAL_SD_CardStateTypeDef state = HAL_SD_GetCardState(drv->hsd);
    if (state == HAL_SD_CARD_TRANSFER) {
      break; // 正常完成
    }
    // 如果突然拔卡，捕捉到异常状态，立刻跳出防止死机
    if (state == HAL_SD_CARD_ERROR || state == (HAL_SD_CardStateTypeDef)0x0FU ||
        state == 0x00U) {
      HAL_SD_Abort(drv->hsd);
      return -3;
    }
    // 超时保护
    if ((HAL_GetTick() - tickstart) >= SD_TIMEOUT) {
      HAL_SD_Abort(drv->hsd);
      return -4;
    }
  }

  return 0;
}

static int sdmmc_drv_control(storage_device_t *self, int cmd, void *arg) {
  // 扩展接口暂空，可根据需要处理同步(Sync)指令或查询卡健康状态
  return 0;
}

static int sdmmc_drv_get_info(storage_device_t *self, storage_info_t *info) {
  if (info != NULL) {
    *info = self->info;
    return 0;
  }
  return -1;
}

/* ================== 外部通知逻辑 ================== */

void sdmmc_storage_drv_on_interrupt(storage_device_t *self, bool is_inserted) {
  if (!self) return;
  
  dev_event_t event = is_inserted ? DEVICE_EVENT_INSERTED : DEVICE_EVENT_REMOVED;
  if (self->drv_cb) {
    self->drv_cb(self, event, self->user_data);
  }
}

/* ================== 函数指针表与实例化 ================== */

static const storage_ops_t s_sdmmc_ops = {.init = sdmmc_drv_init,
                                          .deinit = sdmmc_drv_deinit,
                                          .get_status = sdmmc_drv_get_status,
                                          .read = sdmmc_drv_read,
                                          .write = sdmmc_drv_write,
                                          .erase = sdmmc_drv_erase,
                                          .control = sdmmc_drv_control,
                                          .get_info = sdmmc_drv_get_info};

static sdmmc_storage_device_t s_sd_instance;

storage_device_t *sdmmc_storage_drv_get(const SD_HandleTypeDef *hsd) {
  s_sd_instance.base.ops = &s_sdmmc_ops;
  s_sd_instance.hsd = (SD_HandleTypeDef *)hsd;
  return &s_sd_instance.base;
}
