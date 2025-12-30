#include "mt29f4g08.h"
#include "elog.h"
#include "sys.h"
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "NAND"
#define NAND_MEMSOURCE SYS_MEM_INTERNAL

// MT29F4G08 Geometry
#define NAND_PAGE_SIZE 2048
#define NAND_BLOCK_SIZE (128 * 1024)
#define NAND_BLOCK_COUNT 4096 // 4Gb = 512MB = 4096 blocks * 128KB
#define NAND_PAGE_PER_BLOCK 64

static int mt29f_init(block_device_t *const self) {
  mt29f4g08_t *dev = (mt29f4g08_t *)self;
  if (!dev || !dev->adapter || !dev->adapter->ops->init) {
    return -1;
  }

  log_i("Initializing MT29F4G08...");
  if (dev->adapter->ops->init(dev->adapter) != 0) {
    log_e("Adapter init failed");
    return -1;
  }

  // Reset device
  if (dev->adapter->ops->reset) {
    dev->adapter->ops->reset(dev->adapter);
  }

  // Read ID
  uint32_t id = 0;
  if (dev->adapter->ops->read_id(dev->adapter, &id) == 0) {
    log_i("NAND ID: 0x%08X", id);
    // Optional: Verify ID
  }

  // Set geometry info
  dev->info.capacity = NAND_BLOCK_COUNT * NAND_BLOCK_SIZE;
  dev->info.block_size = NAND_BLOCK_SIZE;
  dev->info.sector_size = NAND_BLOCK_SIZE; // LFS "block" is erase unit
  dev->info.page_size = NAND_PAGE_SIZE;
  dev->info.prog_unit =
      NAND_PAGE_SIZE; // NAND programming is generally page-based
  dev->info.read_unit = NAND_PAGE_SIZE; // NAND reading is generally page-based
  dev->info.erase_value = 0xFF;

  log_i("NAND Init OK. Size: %d MB", dev->info.capacity / 1024 / 1024);
  return 0;
}

static int mt29f_deinit(block_device_t *const self) { return 0; }

static int mt29f_read(block_device_t *const self, uint32_t addr, uint8_t *buf,
                      size_t size) {
  mt29f4g08_t *dev = (mt29f4g08_t *)self;

  // NAND read must usually align to page or at least handle loose bytes
  // carefully. Assuming LFS asks for aligned chunks if we set read_unit = 2048.
  // But strictly, we should handle arbitrary requests or assert alignment.

  uint32_t block = addr / NAND_BLOCK_SIZE;
  uint32_t offset_in_block = addr % NAND_BLOCK_SIZE;
  uint32_t page =
      block * NAND_PAGE_PER_BLOCK + (offset_in_block / NAND_PAGE_SIZE);
  uint32_t col = offset_in_block % NAND_PAGE_SIZE;

  // Simple implementation: Assume request does not cross page boundary for now
  // or caller respects read_unit.
  // If size > page remaining, we need loop.

  uint32_t remains = size;
  uint8_t *ptr = buf;

  while (remains > 0) {
    uint32_t page_remains = NAND_PAGE_SIZE - col;
    uint32_t chunk = (remains < page_remains) ? remains : page_remains;

    if (dev->adapter->ops->read_page(dev->adapter, page, col, ptr, chunk) !=
        0) {
      log_e("Read failed at page %d col %d", page, col);
      return -1;
    }

    remains -= chunk;
    ptr += chunk;

    if (remains > 0) {
      page++;  // Move to next page
      col = 0; // Start from beginning of next page
    }
  }

  return 0;
}

static int mt29f_program(block_device_t *const self, uint32_t addr,
                         const uint8_t *buf, size_t size) {
  mt29f4g08_t *dev = (mt29f4g08_t *)self;

  // Similar to read, map addr to page/col
  uint32_t block = addr / NAND_BLOCK_SIZE;
  uint32_t offset_in_block = addr % NAND_BLOCK_SIZE;
  uint32_t page =
      block * NAND_PAGE_PER_BLOCK + (offset_in_block / NAND_PAGE_SIZE);
  uint32_t col = offset_in_block % NAND_PAGE_SIZE;

  uint32_t remains = size;
  const uint8_t *ptr = buf;

  while (remains > 0) {
    uint32_t page_remains = NAND_PAGE_SIZE - col;
    uint32_t chunk = (remains < page_remains) ? remains : page_remains;

    if (dev->adapter->ops->program_page(dev->adapter, page, col, ptr, chunk) !=
        0) {
      log_e("Program failed at page %d", page);
      return -1;
    }

    remains -= chunk;
    ptr += chunk;

    if (remains > 0) {
      page++;
      col = 0;
    }
  }

  return 0;
}

static int mt29f_erase(block_device_t *const self, uint32_t addr, size_t size) {
  mt29f4g08_t *dev = (mt29f4g08_t *)self;

  // Address must be block aligned
  if (addr % NAND_BLOCK_SIZE != 0 || size % NAND_BLOCK_SIZE != 0) {
    log_e("Erase addr/size not aligned to block size");
    return -1;
  }

  uint32_t start_block = addr / NAND_BLOCK_SIZE;
  uint32_t count = size / NAND_BLOCK_SIZE;

  for (uint32_t i = 0; i < count; i++) {
    uint32_t block_addr = (start_block + i) * NAND_BLOCK_SIZE; // Logical addr
    // Adapter expect block index or address? Interface says "block address
    // (row)" usually Ideally adapter erase_block takes the block index or row
    // address of the first page of the block. Usually Row Address = Block Index
    // * PagesPerBlock. Let's pass the Row Address of the block start.
    uint32_t row_addr = (start_block + i) * NAND_PAGE_PER_BLOCK;

    if (dev->adapter->ops->erase_block(dev->adapter, row_addr) != 0) {
      log_e("Erase block %d failed", start_block + i);
      return -1;
    }
  }

  return 0;
}

static int mt29f_get_info(block_device_t *const self, block_dev_info_t *info) {
  mt29f4g08_t *dev = (mt29f4g08_t *)self;
  if (info) {
    memcpy(info, &dev->info, sizeof(block_dev_info_t));
    return 0;
  }
  return -1;
}

static int mt29f_sync(block_device_t *const self) {
  // NAND usually serves sync by wait_busy after operations
  return 0;
}

static const block_device_ops_t mt29f_ops = {
    .init = mt29f_init,
    .deinit = mt29f_deinit,
    .read = mt29f_read,
    .program = mt29f_program,
    .erase = mt29f_erase,
    .get_info = mt29f_get_info,
    .sync = mt29f_sync,
};

block_device_t *mt29f4g08_create(mt29f_adapter_t *adapter) {
  mt29f4g08_t *dev =
      (mt29f4g08_t *)sys_malloc(NAND_MEMSOURCE, sizeof(mt29f4g08_t));
  if (dev) {
    memset(dev, 0, sizeof(mt29f4g08_t));
    dev->parent.ops = &mt29f_ops;
    dev->adapter = adapter;
    return &dev->parent;
  }
  return NULL;
}

void mt29f4g08_destroy(block_device_t *dev) {
  if (dev) {
    sys_free(NAND_MEMSOURCE, dev);
  }
}
