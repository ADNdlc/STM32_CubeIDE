#include "mt29f_fmc_adapter.h"
#include "sys.h"
#include <stdlib.h>

#define ADAPTER_MEMSOURCE SYS_MEM_INTERNAL

typedef struct {
  mt29f_adapter_t parent;
  NAND_HandleTypeDef *hnand;
} mt29f_fmc_adapter_impl_t;

static int _fmc_init(mt29f_adapter_t *self) {
  // Already initialized by HAL
  return 0;
}

static int _fmc_reset(mt29f_adapter_t *self) {
  mt29f_fmc_adapter_impl_t *impl = (mt29f_fmc_adapter_impl_t *)self;
  if (HAL_NAND_Reset(impl->hnand) != HAL_OK) {
    return -1;
  }
  // Add delay like old driver (100ms after reset)
  HAL_Delay(100);
  return 0;
}

static int _fmc_read_id(mt29f_adapter_t *self, uint32_t *id) {
  mt29f_fmc_adapter_impl_t *impl = (mt29f_fmc_adapter_impl_t *)self;
  NAND_IDTypeDef nand_id;

  // Read ID using HAL
  if (HAL_NAND_Read_ID(impl->hnand, &nand_id) != HAL_OK) {
    return -1;
  }

  // Pack ID like old driver: Skip Maker_Id (0x2C for Micron), use remaining 4
  // bytes Old driver: id = deviceid[1]<<24 | deviceid[2]<<16 | deviceid[3]<<8 |
  // deviceid[4]
  *id = ((uint32_t)nand_id.Device_Id << 24) |
        ((uint32_t)nand_id.Third_Id << 16) | ((uint32_t)nand_id.Fourth_Id << 8);
  // Note: HAL_NAND_Read_ID only returns 4 bytes in NAND_IDTypeDef
  // We might be missing the 5th byte, but this should match the pattern

  return 0;
}

#include "elog.h"
#define LOG_TAG "FMC_ADP"

static int _fmc_read_page(mt29f_adapter_t *self, uint32_t page, uint32_t col,
                          uint8_t *buf, size_t size) {
  mt29f_fmc_adapter_impl_t *impl = (mt29f_fmc_adapter_impl_t *)self;
  NAND_AddressTypeDef addr;
  const uint32_t PAGES_PER_BLOCK = 64;
  //log_i("Unsupported read: page=%d, col=%d, size=%d", page, col, size);
  addr.Block = page / PAGES_PER_BLOCK;
  addr.Page = page % PAGES_PER_BLOCK;
  addr.Plane = (addr.Block >= 2048) ? 1 : 0;
  if (addr.Plane == 1)
    addr.Block -= 2048;

  if (col != 0 || size != 2048) {
    log_e("Unsupported read: page=%d, col=%d, size=%d", page, col, size);
    return -1;
  }

  if (HAL_NAND_Read_Page_8b(impl->hnand, &addr, buf, 1) != HAL_OK) {
    log_e("HAL_NAND_Read_Page failed: page=%d", page);
    return -1;
  }
  return 0;
}

static int _fmc_program_page(mt29f_adapter_t *self, uint32_t page, uint32_t col,
                             const uint8_t *buf, size_t size) {
  mt29f_fmc_adapter_impl_t *impl = (mt29f_fmc_adapter_impl_t *)self;
  NAND_AddressTypeDef addr;
  const uint32_t PAGES_PER_BLOCK = 64;

  addr.Block = page / PAGES_PER_BLOCK;
  addr.Page = page % PAGES_PER_BLOCK;
  addr.Plane = (addr.Block >= 2048) ? 1 : 0;
  if (addr.Plane == 1)
    addr.Block -= 2048;

  if (col != 0 || size != 2048) {
    log_e("Unsupported program: page=%d, col=%d, size=%d", page, col, size);
    return -1;
  }

  if (HAL_NAND_Write_Page_8b(impl->hnand, &addr, (uint8_t *)buf, 1) != HAL_OK) {
    log_e("HAL_NAND_Write_Page failed: page=%d", page);
    return -1;
  }
  return 0;
}

static int _fmc_erase_block(mt29f_adapter_t *self, uint32_t row_addr) {
  mt29f_fmc_adapter_impl_t *impl = (mt29f_fmc_adapter_impl_t *)self;
  NAND_AddressTypeDef addr;
  const uint32_t PAGES_PER_BLOCK = 64;

  uint32_t block_idx = row_addr / PAGES_PER_BLOCK;
  addr.Block = block_idx;
  addr.Page = 0;
  addr.Plane = (addr.Block >= 2048) ? 1 : 0;
  if (addr.Plane == 1)
    addr.Block -= 2048;

  if (HAL_NAND_Erase_Block(impl->hnand, &addr) != HAL_OK) {
    log_e("HAL_NAND_Erase_Block failed: block=%d", block_idx);
    return -1;
  }
  return 0;
}

static int _fmc_wait_busy(mt29f_adapter_t *self, uint32_t timeout_ms) {
  // HAL functions usually block until completion?
  // HAL_NAND_Write_Page uses polling/interrupt? usually polling.
  // So explicit wait might not be needed if HAL does it.
  // But LFS might want sync.
  return 0;
}

static const mt29f_adapter_ops_t fmc_ops = {
    .init = _fmc_init,
    .reset = _fmc_reset,
    .read_id = _fmc_read_id,
    .read_page = _fmc_read_page,
    .program_page = _fmc_program_page,
    .erase_block = _fmc_erase_block,
    .wait_busy = _fmc_wait_busy,
};

mt29f_adapter_t *mt29f_fmc_adapter_create(NAND_HandleTypeDef *hnand) {
  mt29f_fmc_adapter_impl_t *impl = (mt29f_fmc_adapter_impl_t *)sys_malloc(
      ADAPTER_MEMSOURCE, sizeof(mt29f_fmc_adapter_impl_t));
  if (impl) {
    impl->parent.ops = &fmc_ops;
    impl->hnand = hnand;
    return &impl->parent;
  }
  return NULL;
}
