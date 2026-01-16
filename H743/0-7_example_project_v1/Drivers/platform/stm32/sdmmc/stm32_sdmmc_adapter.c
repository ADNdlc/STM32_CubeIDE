#include "stm32_sdmmc_adapter.h"
#include "elog.h"
#include "sdmmc.h"
#include "stm32h7xx_hal.h"
#include "sys.h"
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "SDMMC_Adapter"
#define ADAPTER_MEMSOURCE SYS_MEM_INTERNAL

typedef struct {
  sdcard_adapter_t parent;
  SD_HandleTypeDef *hsd;
} stm32_sdmmc_adapter_impl_t;

// 前向声明
static int stm32_sdmmc_reset(sdcard_adapter_t *self);

static int stm32_sdmmc_init(sdcard_adapter_t *self) {
  stm32_sdmmc_adapter_impl_t *impl = (stm32_sdmmc_adapter_impl_t *)self;

  // 如果已经READY，快速返回
  if (impl->hsd->State == HAL_SD_STATE_READY) {
    return 0;
  }

  // 关键改进: 先完全重置硬件
  log_i("SDMMC state is %d, resetting hardware...", impl->hsd->State);
  stm32_sdmmc_reset(self);

  // 重新初始化
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd1.Init.ClockDiv = SDMMC_NSpeed_CLK_DIV;

  if (HAL_SD_Init(&hsd1) != HAL_OK) {
    log_e("HAL_SD_Init failed after reset");
    return -1;
  }

  log_i("SDMMC init success");
  return 0;
}

static int stm32_sdmmc_deinit(sdcard_adapter_t *self) {
  stm32_sdmmc_adapter_impl_t *impl = (stm32_sdmmc_adapter_impl_t *)self;

  // 强制反初始化
  HAL_SD_DeInit(impl->hsd);
  log_i("SDMMC deinit completed");

  return 0;
}

static int stm32_sdmmc_read_blocks(sdcard_adapter_t *self, uint32_t block_addr,
                                   uint8_t *buf, uint32_t block_count) {
  stm32_sdmmc_adapter_impl_t *impl = (stm32_sdmmc_adapter_impl_t *)self;
  HAL_StatusTypeDef status;

  log_i("SD Read: block %u, count %u", block_addr, block_count);

  status = HAL_SD_ReadBlocks_DMA(impl->hsd, buf, block_addr, block_count);

  if (status != HAL_OK) {
    log_e("HAL_SD_ReadBlocks_DMA failed: %d, ErrorCode: 0x%08X", status,
          impl->hsd->ErrorCode);
    return -1;
  }

  // 1. Wait for HAL State to be READY (DMA transfer finished)
  uint32_t timeout = 500000;
  while (HAL_SD_GetState(impl->hsd) != HAL_SD_STATE_READY) {
    if (timeout-- == 0) {
      log_e("Wait for HAL READY timeout, ErrorCode: 0x%08X",
            impl->hsd->ErrorCode);
      return -1;
    }
  }

  // 2. Wait for Card to be in TRANSFER state
  timeout = 500000;
  while (HAL_SD_GetCardState(impl->hsd) != HAL_SD_CARD_TRANSFER) {
    if (timeout-- == 0) {
      log_e("Wait for card TRANSFER state timeout");
      return -1;
    }
  }

  // 3. Cache Maintenance: Invalidate D-Cache after DMA read
  if (SCB->CCR & SCB_CCR_DC_Msk) {
    SCB_InvalidateDCache_by_Addr((uint32_t *)buf, block_count * 512);
  }

  return 0;
}

static int stm32_sdmmc_write_blocks(sdcard_adapter_t *self, uint32_t block_addr,
                                    const uint8_t *buf, uint32_t block_count) {
  stm32_sdmmc_adapter_impl_t *impl = (stm32_sdmmc_adapter_impl_t *)self;
  HAL_StatusTypeDef status;

  log_i("SD Write: block %d, count %d", block_addr, block_count);

  // 1. Cache Maintenance: Clean D-Cache before DMA write
  if (SCB->CCR & SCB_CCR_DC_Msk) {
    SCB_CleanDCache_by_Addr((uint32_t *)buf, block_count * 512);
  }

  status = HAL_SD_WriteBlocks_DMA(impl->hsd, (uint8_t *)buf, block_addr,
                                  block_count);

  if (status != HAL_OK) {
    log_e("HAL_SD_WriteBlocks_DMA failed: %d, ErrorCode: 0x%08X", status,
          impl->hsd->ErrorCode);
    return -1;
  }

  // 2. Wait for HAL State to be READY (DMA transfer finished)
  uint32_t timeout = 500000;
  while (HAL_SD_GetState(impl->hsd) != HAL_SD_STATE_READY) {
    if (timeout-- == 0) {
      log_e("Wait for HAL READY timeout, ErrorCode: 0x%08X",
            impl->hsd->ErrorCode);
      return -1;
    }
  }

  // 3. Wait for Card to be in TRANSFER state (Finish programming)
  timeout = 1000000;
  while (HAL_SD_GetCardState(impl->hsd) != HAL_SD_CARD_TRANSFER) {
    if (timeout-- == 0) {
      log_e("Wait for card TRANSFER state timeout");
      return -1;
    }
  }

  return 0;
}

static int stm32_sdmmc_get_info(sdcard_adapter_t *self, sdcard_info_t *info) {
  stm32_sdmmc_adapter_impl_t *impl = (stm32_sdmmc_adapter_impl_t *)self;
  HAL_SD_CardInfoTypeDef card_info;

  if (HAL_SD_GetCardInfo(impl->hsd, &card_info) != HAL_OK) {
    return -1;
  }

  info->block_size = card_info.LogBlockSize;
  info->block_count = card_info.LogBlockNbr;
  info->capacity_mb = (uint32_t)((uint64_t)card_info.LogBlockNbr *
                                 card_info.LogBlockSize / 1024 / 1024);

  return 0;
}

static int stm32_sdmmc_is_ready(sdcard_adapter_t *self) {
  stm32_sdmmc_adapter_impl_t *impl = (stm32_sdmmc_adapter_impl_t *)self;

  HAL_SD_StateTypeDef state = HAL_SD_GetState(impl->hsd);
  if (state == HAL_SD_STATE_RESET || state == HAL_SD_STATE_ERROR) {
    return 0; // Succeed to allow flash_handler to try mounting (init)
  }

  if (state != HAL_SD_STATE_READY) {
    return 1; // Busy or Transferring
  }

  // Check card state electronically
  return (HAL_SD_GetCardState(impl->hsd) == HAL_SD_CARD_TRANSFER) ? 0 : 1;
}

/**
 * @brief 检测SD卡是否物理存在
 * @return 1=存在, 0=不存在, -1=需要重新初始化才能确定
 */
static int stm32_sdmmc_detect(sdcard_adapter_t *self) {
  stm32_sdmmc_adapter_impl_t *impl = (stm32_sdmmc_adapter_impl_t *)self;

  HAL_SD_StateTypeDef state = HAL_SD_GetState(impl->hsd);

  // 已初始化成功，尝试获取卡状态
  if (state == HAL_SD_STATE_READY) {
    HAL_SD_CardStateTypeDef card_state = HAL_SD_GetCardState(impl->hsd);
    if (card_state == HAL_SD_CARD_ERROR) {
      return 0; // 卡不存在或通信错误
    }
    return 1; // 卡存在
  }

  // 未初始化状态，返回-1表示需要尝试初始化来确定
  return -1;
}

/**
 * @brief 完整硬件重置(GPIO/外设/时钟)
 */
static int stm32_sdmmc_reset(sdcard_adapter_t *self) {
  stm32_sdmmc_adapter_impl_t *impl = (stm32_sdmmc_adapter_impl_t *)self;

  // 1. 强制反初始化HAL
  HAL_SD_DeInit(impl->hsd);

  // 2. 复位SDMMC外设时钟
  __HAL_RCC_SDMMC1_FORCE_RESET();
  HAL_Delay(2);
  __HAL_RCC_SDMMC1_RELEASE_RESET();

  // 3. 延时等待卡稳定
  HAL_Delay(50);

  log_i("SDMMC hardware reset completed");
  return 0;
}

static const sdcard_adapter_ops_t stm32_sdmmc_ops = {
    .init = stm32_sdmmc_init,
    .deinit = stm32_sdmmc_deinit,
    .read_blocks = stm32_sdmmc_read_blocks,
    .write_blocks = stm32_sdmmc_write_blocks,
    .get_info = stm32_sdmmc_get_info,
    .is_ready = stm32_sdmmc_is_ready,
    .detect = stm32_sdmmc_detect,
    .reset = stm32_sdmmc_reset,
};

sdcard_adapter_t *stm32_sdmmc_adapter_create(SD_HandleTypeDef *hsd) {
  if (!hsd)
    return NULL;

  stm32_sdmmc_adapter_impl_t *impl = (stm32_sdmmc_adapter_impl_t *)sys_malloc(
      ADAPTER_MEMSOURCE, sizeof(stm32_sdmmc_adapter_impl_t));
  if (impl) {
    memset(impl, 0, sizeof(stm32_sdmmc_adapter_impl_t));
    impl->parent.ops = &stm32_sdmmc_ops;
    impl->hsd = hsd;
    return &impl->parent;
  }
  return NULL;
}

void stm32_sdmmc_adapter_destroy(sdcard_adapter_t *adapter) {
  if (adapter) {
    sys_free(ADAPTER_MEMSOURCE, adapter);
  }
}
