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

  // 假设 STM32CubeMX 已经在 main 中调用了 MX_SDMMC1_SD_Init()
  // 此处仅验证卡是否正常并获取信息
  if (HAL_SD_GetCardState(drv->hsd) == HAL_SD_CARD_ERROR) {
    return -1;
  }

  if (HAL_SD_GetCardInfo(drv->hsd, &card_info) != HAL_OK) {
    return -2;
  }

  // 填充设备缓存信息
  self->info.type = STORAGE_TYPE_SD_CARD;
  self->info.read_size = SD_BLOCK_SIZE;
  self->info.write_size = SD_BLOCK_SIZE;
  self->info.erase_size = SD_BLOCK_SIZE;

  // 注意：如果是大于 4GB 的 SDHC/SDXC 卡，card_info.BlockNbr * 512 可能会导致
  // uint32_t 溢出。 如果要支持 32GB 甚至更大的卡，storage_interface.h 中的
  // total_size 需为 uint64_t
  self->info.total_size = card_info.BlockNbr * card_info.BlockSize;

  // 获取卡的 CID (作为设备唯一 ID)
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
  // 如果 HAL 句柄本身处于 RESET 状态，说明还没经过 Init，无法确定卡在不在
  if (drv->hsd->State == HAL_SD_STATE_RESET) {
    return STORAGE_STATUS_NOT_INIT;
  }
  // 主动发送 CMD13 查询卡状态
  // HAL_SD_GetCardState 内部会发送 CMD13 并等待响应
  HAL_SD_CardStateTypeDef state = HAL_SD_GetCardState(drv->hsd);
  // 如果返回错误，通常是因为总线超时（卡被拔掉了）
  if (state == HAL_SD_CARD_ERROR) {
    // 进一步确认：尝试重新检测总线
    // 如果卡真的不在，这里会持续报错
    return STORAGE_STATUS_OFFLINE;
  }
  // 正常状态
  if (state == HAL_SD_CARD_TRANSFER) {
    return STORAGE_STATUS_OK;
  }
  return STORAGE_STATUS_BUSY;
}

static int sdmmc_drv_read(storage_device_t *self, uint32_t addr, uint8_t *buf,
                          size_t len) {
  sdmmc_storage_device_t *drv = (sdmmc_storage_device_t *)self;

  // 检查参数是否为块对齐 (SD 卡必须按 512 字节边界读写)
  if ((addr % SD_BLOCK_SIZE) != 0 || (len % SD_BLOCK_SIZE) != 0) {
    return -1; // 错误：非对齐访问
  }

  uint32_t block_idx = addr / SD_BLOCK_SIZE;
  uint32_t block_cnt = len / SD_BLOCK_SIZE;

  // 此处使用轮询阻塞读取。如果对性能要求高，应改用 HAL_SD_ReadBlocks_DMA / IT
  if (HAL_SD_ReadBlocks(drv->hsd, buf, block_idx, block_cnt, SD_TIMEOUT) !=
      HAL_OK) {
    return -2;
  }

  // 等待传输彻底完成
  while (HAL_SD_GetCardState(drv->hsd) != HAL_SD_CARD_TRANSFER) {
    // 可选：在此处添加 osDelay 或超时处理
  }

  return 0;
}

static int sdmmc_drv_write(storage_device_t *self, uint32_t addr,
                           const uint8_t *data, size_t len) {
  sdmmc_storage_device_t *drv = (sdmmc_storage_device_t *)self;

  if ((addr % SD_BLOCK_SIZE) != 0 || (len % SD_BLOCK_SIZE) != 0) {
    return -1;
  }

  uint32_t block_idx = addr / SD_BLOCK_SIZE;
  uint32_t block_cnt = len / SD_BLOCK_SIZE;

  // 强制丢弃 const，因为 HAL 库接口不带 const
  if (HAL_SD_WriteBlocks(drv->hsd, (uint8_t *)data, block_idx, block_cnt,
                         SD_TIMEOUT) != HAL_OK) {
    return -2;
  }

  while (HAL_SD_GetCardState(drv->hsd) != HAL_SD_CARD_TRANSFER) {
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
    return -2;
  }

  while (HAL_SD_GetCardState(drv->hsd) != HAL_SD_CARD_TRANSFER) {
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
