#include "w25qxx.h"
#include "sys.h"
#include <stdlib.h>
#include <string.h>

#define W25Q_MEMSOURCE SYS_MEM_INTERNAL

// W25Qxx Manufacturer and Device IDs
#define WINBOND_MANUFACTURER_ID 0xEF
#define W25Q_MEMORY_TYPE        0x40

// Specific device IDs
#define W25Q10_ID 0xEF10
#define W25Q20_ID 0xEF11
#define W25Q40_ID 0xEF12
#define W25Q80_ID 0xEF13
#define W25Q16_ID 0xEF14
#define W25Q32_ID 0xEF15
#define W25Q64_ID 0xEF16
#define W25Q128_ID 0xEF17
#define W25Q256_ID 0xEF18

static int w25q_init(block_device_t *const self) {
  w25qxx_t *dev = (w25qxx_t *)self;
  if (dev->adapter && dev->adapter->ops->init) {
    if (dev->adapter->ops->init(dev->adapter) != 0)
      return -1;
  }

  // 读取ID
  uint32_t id = 0;
  if (dev->adapter->ops->read_id(dev->adapter, &id) != 0)
    return -1;

  // 验证ID信息
  uint8_t manuf_id = (id >> 16) & 0xFF;
  uint8_t mem_type = (id >> 8) & 0xFF;
  uint8_t cap_id = id & 0xFF;
  
  if (manuf_id != WINBOND_MANUFACTURER_ID || mem_type != W25Q_MEMORY_TYPE) {
    return -1; // Not a W25Q series flash
  }

  // 根据ID信息设定 block_device信息
  switch (cap_id) {
    case 0x10: // W25Q10 (1Mbit = 128KB)
      dev->info.capacity = 128 * 1024;      // 128KB
      dev->info.block_size = 64 * 1024;     // 64KB block
      dev->info.sector_size = 4 * 1024;     // 4KB sector
      dev->info.page_size = 256;            // 256B page
      break;
    case 0x11: // W25Q20 (2Mbit = 256KB)
      dev->info.capacity = 256 * 1024;      // 256KB
      dev->info.block_size = 64 * 1024;
      dev->info.sector_size = 4 * 1024;
      dev->info.page_size = 256;
      break;
    case 0x12: // W25Q40 (4Mbit = 512KB)
      dev->info.capacity = 512 * 1024;      // 512KB
      dev->info.block_size = 64 * 1024;
      dev->info.sector_size = 4 * 1024;
      dev->info.page_size = 256;
      break;
    case 0x13: // W25Q80 (8Mbit = 1MB)
      dev->info.capacity = 1 * 1024 * 1024; // 1MB
      dev->info.block_size = 64 * 1024;
      dev->info.sector_size = 4 * 1024;
      dev->info.page_size = 256;
      break;
    case 0x14: // W25Q16 (16Mbit = 2MB)
      dev->info.capacity = 2 * 1024 * 1024; // 2MB
      dev->info.block_size = 64 * 1024;
      dev->info.sector_size = 4 * 1024;
      dev->info.page_size = 256;
      break;
    case 0x15: // W25Q32 (32Mbit = 4MB)
      dev->info.capacity = 4 * 1024 * 1024; // 4MB
      dev->info.block_size = 64 * 1024;
      dev->info.sector_size = 4 * 1024;
      dev->info.page_size = 256;
      break;
    case 0x16: // W25Q64 (64Mbit = 8MB)
      dev->info.capacity = 8 * 1024 * 1024; // 8MB
      dev->info.block_size = 64 * 1024;
      dev->info.sector_size = 4 * 1024;
      dev->info.page_size = 256;
      break;
    case 0x17: // W25Q128 (128Mbit = 16MB)
      dev->info.capacity = 16 * 1024 * 1024; // 16MB
      dev->info.block_size = 64 * 1024;
      dev->info.sector_size = 4 * 1024;
      dev->info.page_size = 256;
      break;
    case 0x18: // W25Q256 (256Mbit = 32MB)
      dev->info.capacity = 32 * 1024 * 1024; // 32MB
      dev->info.block_size = 64 * 1024;
      dev->info.sector_size = 4 * 1024;
      dev->info.page_size = 256;
      break;
    default:
      return -1; // Unsupported device
  }

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

/**
 * @brief 同步
 * 
 * @param self  w25qxx设备句柄 
 * @return int  0 成功，-1 失败
 */
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

/**
 * @brief 创建W25Q系列flash设备
 * 
 * @param adapter           传输适配器
 * @return block_device_t*  设备句柄 
 */
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
