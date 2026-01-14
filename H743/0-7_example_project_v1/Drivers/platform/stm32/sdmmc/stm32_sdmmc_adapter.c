#include "stm32_sdmmc_adapter.h"
#include "elog.h"
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

static int stm32_sdmmc_init(sdcard_adapter_t *self) {
  stm32_sdmmc_adapter_impl_t *impl = (stm32_sdmmc_adapter_impl_t *)self;

  // HAL_SD_Init is usually called by CubeMX generated code,
  // but we can check the state here if needed.
  if (impl->hsd->State == HAL_SD_STATE_RESET) {
    log_w("SDMMC state is RESET, attempting HAL_SD_Init...");
    // In a real scenario, the initialization parameters should be correctly set
    // in MX_SDMMC1_SD_Init() If we need to re-init here, we'd need the init
    // struct values.
  }

  return (impl->hsd->State != HAL_SD_STATE_ERROR) ? 0 : -1;
}

static int stm32_sdmmc_deinit(sdcard_adapter_t *self) { return 0; }

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
  if (HAL_SD_GetState(impl->hsd) != HAL_SD_STATE_READY) {
    return 1;
  }
  return (HAL_SD_GetCardState(impl->hsd) == HAL_SD_CARD_TRANSFER) ? 0 : 1;
}

static const sdcard_adapter_ops_t stm32_sdmmc_ops = {
    .init = stm32_sdmmc_init,
    .deinit = stm32_sdmmc_deinit,
    .read_blocks = stm32_sdmmc_read_blocks,
    .write_blocks = stm32_sdmmc_write_blocks,
    .get_info = stm32_sdmmc_get_info,
    .is_ready = stm32_sdmmc_is_ready,
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
