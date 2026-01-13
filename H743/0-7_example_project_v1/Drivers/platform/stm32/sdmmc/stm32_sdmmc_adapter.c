#include "stm32_sdmmc_adapter.h"
#include "elog.h"
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

  // Use Polling read as in the old driver
  status = HAL_SD_ReadBlocks(impl->hsd, buf, block_addr, block_count, 10000);
  if (status != HAL_OK) {
    log_e("HAL_SD_ReadBlocks failed: %d", status);
    return -1;
  }

  // Wait for transfer to complete
  uint32_t timeout = 1000000;
  while (HAL_SD_GetCardState(impl->hsd) != HAL_SD_CARD_TRANSFER) {
    if (timeout-- == 0) {
      log_e("Wait for read transfer timeout");
      return -1;
    }
  }

  return 0;
}

static int stm32_sdmmc_write_blocks(sdcard_adapter_t *self, uint32_t block_addr,
                                    const uint8_t *buf, uint32_t block_count) {
  stm32_sdmmc_adapter_impl_t *impl = (stm32_sdmmc_adapter_impl_t *)self;
  HAL_StatusTypeDef status;

  status = HAL_SD_WriteBlocks(impl->hsd, (uint8_t *)buf, block_addr,
                              block_count, 10000);
  if (status != HAL_OK) {
    log_e("HAL_SD_WriteBlocks failed: %d", status);
    return -1;
  }

  // Wait for transfer to complete
  uint32_t timeout = 1000000;
  while (HAL_SD_GetCardState(impl->hsd) != HAL_SD_CARD_TRANSFER) {
    if (timeout-- == 0) {
      log_e("Wait for write transfer timeout");
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
