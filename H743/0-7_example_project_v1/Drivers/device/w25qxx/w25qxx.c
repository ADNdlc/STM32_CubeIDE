#include "w25qxx.h"
#include "sys.h"
#include <stdlib.h>
#include <string.h>

#define W25Q_MEMSOURCE SYS_MEM_INTERNAL

static int w25q_init(block_device_t *const self) {
  w25qxx_t *dev = (w25qxx_t *)self;
  if (dev->adapter && dev->adapter->ops->init) {
    if (dev->adapter->ops->init(dev->adapter) != 0)
      return -1;
  }

  // Read ID to verify and setup info
  uint32_t id = 0;
  if (dev->adapter->ops->read_id(dev->adapter, &id) != 0)
    return -1;

  // Basic setup for W25Q256 (simplified, should decode ID)
  dev->info.capacity = 32 * 1024 * 1024; // 32MB
  dev->info.block_size = 64 * 1024;
  dev->info.sector_size = 4 * 1024;
  dev->info.page_size = 256;
  dev->info.erase_value = 0xFF;

  return 0;
}

static int w25q_deinit(block_device_t *const self) { return 0; }

static int w25q_read(block_device_t *const self, uint32_t addr,
                     uint8_t *out_buf, size_t size) {
  w25qxx_t *dev = (w25qxx_t *)self;
  return dev->adapter->ops->read(dev->adapter, addr, out_buf, size);
}

static int w25q_program(block_device_t *const self, uint32_t addr,
                        const uint8_t *in_buf, size_t size) {
  w25qxx_t *dev = (w25qxx_t *)self;
  uint32_t current_addr = addr;
  uint32_t end_addr = addr + size;
  const uint8_t *data_ptr = in_buf;

  while (current_addr < end_addr) {
    uint32_t page_offset = current_addr % dev->info.page_size;
    uint32_t bytes_left_in_page = dev->info.page_size - page_offset;
    uint32_t chunk_size = (end_addr - current_addr) < bytes_left_in_page
                              ? (end_addr - current_addr)
                              : bytes_left_in_page;

    if (dev->adapter->ops->write_enable(dev->adapter) != 0)
      return -1;
    if (dev->adapter->ops->program_page(dev->adapter, current_addr, data_ptr,
                                        chunk_size) != 0)
      return -1;
    if (dev->adapter->ops->wait_busy(dev->adapter, 1000) != 0)
      return -1;

    current_addr += chunk_size;
    data_ptr += chunk_size;
  }
  return 0;
}

static int w25q_erase(block_device_t *const self, uint32_t addr, size_t size) {
  w25qxx_t *dev = (w25qxx_t *)self;
  // Simple implementation: erase sectors
  // Needs alignment check in real world
  uint32_t current_addr = addr;
  uint32_t end_addr = addr + size;

  while (current_addr < end_addr) {
    if (dev->adapter->ops->write_enable(dev->adapter) != 0)
      return -1;
    if (dev->adapter->ops->erase_sector(dev->adapter, current_addr) != 0)
      return -1;
    if (dev->adapter->ops->wait_busy(dev->adapter, 1000) != 0)
      return -1;
    current_addr += dev->info.sector_size;
  }
  return 0;
}

static int w25q_get_info(block_device_t *const self, block_dev_info_t *info) {
  w25qxx_t *dev = (w25qxx_t *)self;
  if (info) {
    memcpy(info, &dev->info, sizeof(block_dev_info_t));
    return 0;
  }
  return -1;
}

static int w25q_sync(block_device_t *const self) {
  w25qxx_t *dev = (w25qxx_t *)self;
  return dev->adapter->ops->wait_busy(dev->adapter, 1000);
}

static const block_device_ops_t w25q_ops = {
    .init = w25q_init,
    .deinit = w25q_deinit,
    .read = w25q_read,
    .program = w25q_program,
    .erase = w25q_erase,
    .get_info = w25q_get_info,
    .sync = w25q_sync,
};

block_device_t *w25qxx_create(w25q_adapter_t *adapter) {
  w25qxx_t *dev = (w25qxx_t *)sys_malloc(W25Q_MEMSOURCE, sizeof(w25qxx_t));
  if (dev) {
    dev->parent.ops = &w25q_ops;
    dev->adapter = adapter;
    return &dev->parent;
  }
  return NULL;
}

void w25qxx_destroy(block_device_t *dev) {
  if (dev) {
    sys_free(W25Q_MEMSOURCE, dev);
  }
}
