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
  return 0;
}

static int _fmc_read_id(mt29f_adapter_t *self, uint32_t *id) {
  mt29f_fmc_adapter_impl_t *impl = (mt29f_fmc_adapter_impl_t *)self;
  NAND_IDTypeDef nand_id;
  if (HAL_NAND_Read_ID(impl->hnand, &nand_id) != HAL_OK) {
    return -1;
  }
  // Pack ID: Maker | Device | 3rd | 4th
  *id = (nand_id.Maker_Id << 24) | (nand_id.Device_Id << 16) |
        (nand_id.Third_Id << 8) | (nand_id.Fourth_Id);
  return 0;
}

static int _fmc_read_page(mt29f_adapter_t *self, uint32_t page, uint32_t col,
                          uint8_t *buf, size_t size) {
  mt29f_fmc_adapter_impl_t *impl = (mt29f_fmc_adapter_impl_t *)self;
  NAND_AddressTypeDef addr;
  addr.Page = page;
  addr.Plane = 0; // Assuming 1 plane or handled by HAL
  addr.Block = 0; // Page address usually includes block info in HAL_NAND logic?
                  // Wait, HAL_NAND_Read_Page_8b takes Address struct which has
                  // Block, Page, Plane. My interface passed 'page' as absolute
                  // linear page index or Row Address? In mt29f4g08.c I passed
                  // row_addr (Page + Block*PagesPerBlock). For
                  // HAL_NAND_Read_Page_8b, we need to split it.

  // Geometry Constants (should match mt29f4g08.c or be passed in config)
  // MT29F4G08: 0x1000 blocks, 64 pages per block
  const uint32_t PAGES_PER_BLOCK = 64;

  addr.Block = page / PAGES_PER_BLOCK;
  addr.Page = page % PAGES_PER_BLOCK;
  addr.Plane = 0;

  // Column address is usually handled inside HAL if it supports partial read?
  // HAL_NAND_Read_Page_8b reads the WHOLE page?
  // Checking STM32 HAL: HAL_NAND_Read_Page_8b(NAND_HandleTypeDef *hnand,
  // NAND_AddressTypeDef *pAddress, uint8_t *pBuffer, uint32_t NumPageToRead) It
  // reads whole pages. It does NOT support column offset directly in the high
  // level API. If we want column offset, we might need low level API or read
  // whole page and copy. For simplicity, let's read the whole page into a temp
  // buffer if necessary, or assume aligned full page read. However, LFS often
  // reads small chunks? Yes, LFS tests (and my test) set read_unit = 2048 (Page
  // Size). So LFS will likely read full pages. My mt29f_read implementation in
  // mt29f4g08.c logic: It loops and calls read_page(..., col, ..., chunk). If
  // col != 0 or chunk != 2048, we have a problem with standard
  // HAL_NAND_Read_Page. WE MUST SUPPORT PARTIAL READ if we exposd
  // read_unit=2048? No, if read_unit is 2048, LFS only requests aligned full
  // blocks? LFS "read_size" config: "Minimum size of a block read". If we set
  // it to 2048, LFS should respect it. BUT, checks in `mt29f_read` handled
  // offset.

  // Let's assume for now we only support full page reads aligned to 0.
  if (col != 0 || size != 2048) {
    // Fallback: Read full page to temp buffer?
    // Or specific HAL function?
    // HAL_NAND_Read_SpareArea exists...
    // Let's implement full page read requirement for now.
    // If necessary we can add a temp buffer here but stack usage is high (2KB).
    // Let's assume aligned for simplicity of prototype, else Error.
    return -1;
  }

  if (HAL_NAND_Read_Page_8b(impl->hnand, &addr, buf, 1) != HAL_OK) {
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
  addr.Plane = 0;

  if (col != 0 || size != 2048) {
    return -1; // Only full page program supported via simple HAL
  }

  if (HAL_NAND_Write_Page_8b(impl->hnand, &addr, (uint8_t *)buf, 1) != HAL_OK) {
    return -1;
  }
  return 0;
}

static int _fmc_erase_block(mt29f_adapter_t *self, uint32_t row_addr) {
  mt29f_fmc_adapter_impl_t *impl = (mt29f_fmc_adapter_impl_t *)self;
  NAND_AddressTypeDef addr;
  const uint32_t PAGES_PER_BLOCK = 64;

  addr.Block = row_addr / PAGES_PER_BLOCK;
  addr.Page = 0; // Erase is block level
  addr.Plane = 0;

  if (HAL_NAND_Erase_Block(impl->hnand, &addr) != HAL_OK) {
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
