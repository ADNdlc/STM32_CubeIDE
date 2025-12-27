#include "w25qxx.h"
#include "../../interface/block_device.h" // Help lint/compiler find it
#include "elog.h"
#include "sys.h"
#include <stdlib.h>
#include <string.h>


#define LOG_TAG "W25Q"

#define W25Q_MEMSOURCE SYS_MEM_INTERNAL

// W25Qxx Manufacturer and Device IDs
#define WINBOND_MANUFACTURER_ID 0xEF
#define W25Q_MEMORY_TYPE 0x40

static int w25q_init(block_device_t *const self) {
  log_d("Starting W25Q initialization");
  w25qxx_t *dev = (w25qxx_t *)self;
  log_d("Device structure cast successful");
  
  if (dev->adapter && dev->adapter->ops->init) {
    log_d("Calling adapter init function");
    if (dev->adapter->ops->init(dev->adapter) != 0) {
      log_e("Adapter init failed");
      return -1;
    }
    log_i("Adapter init successful");
  } else {
    log_w("Adapter or init function is NULL");
    if (!dev->adapter) {
      log_e("Adapter is NULL");
      return -1;
    }
    if (!dev->adapter->ops) {
      log_e("Adapter ops is NULL");
      return -1;
    }
    if (!dev->adapter->ops->init) {
      log_e("Adapter init function is NULL");
      return -1;
    }
  }

  // 1. Read ID
  log_d("Attempting to read Flash ID");
  uint32_t id = 0;
  if (dev->adapter->ops->read_id(dev->adapter, &id) != 0) {
    log_e("Read ID failed");
    return -1;
  }
  log_i("Found Flash ID: 0x%06X", id);

  // 2. Parse ID
  uint8_t manuf_id = (id >> 16) & 0xFF;
  uint8_t mem_type = (id >> 8) & 0xFF;
  uint8_t cap_id = id & 0xFF;

  log_d("Parsed Flash ID - Manufacturer: 0x%02X, Memory Type: 0x%02X, Capacity ID: 0x%02X", manuf_id, mem_type, cap_id);
  if (manuf_id != WINBOND_MANUFACTURER_ID || mem_type != W25Q_MEMORY_TYPE) {
    log_w("Unexpected Chip: Manuf 0x%02X, Type 0x%02X", manuf_id, mem_type);
  }

  // 3. Set Geometry
  log_d("Setting flash geometry based on capacity ID: 0x%02X", cap_id);
  switch (cap_id) {
  case 0x10:
    dev->info.capacity = 128 * 1024;
    break;
  case 0x11:
    dev->info.capacity = 256 * 1024;
    break;
  case 0x12:
    dev->info.capacity = 512 * 1024;
    break;
  case 0x13:
    dev->info.capacity = 1024 * 1024;
    break;
  case 0x14:
    dev->info.capacity = 2 * 1024 * 1024;
    break;
  case 0x15:
    dev->info.capacity = 4 * 1024 * 1024;
    break;
  case 0x16:
    dev->info.capacity = 8 * 1024 * 1024;
    break;
  case 0x17:
    dev->info.capacity = 16 * 1024 * 1024;
    break;
  case 0x18:
    dev->info.capacity = 32 * 1024 * 1024;
    break;
  default:
    log_e("Unsupported capacity ID: 0x%02X", cap_id);
    return -1;
  }
  dev->info.block_size = 64 * 1024;
  dev->info.sector_size = 4 * 1024;
  dev->info.page_size = 256;
  dev->info.erase_value = 0xFF;

  log_i("Flash Geometry: %d KB capacity", (int)(dev->info.capacity / 1024));
  log_d("W25Q initialization completed successfully");
  return 0;
}

static int w25q_deinit(block_device_t *const self) { 
  log_d("W25Q deinit called");
  return 0; 
}

static int w25q_read(block_device_t *const self, uint32_t addr, uint8_t *buf,
                     size_t size) {
  w25qxx_t *dev = (w25qxx_t *)self;
  log_d("W25Q read called: addr=0x%08X, size=%d", addr, (int)size);
  int result = dev->adapter->ops->read(dev->adapter, addr, buf, size);
  log_d("W25Q read completed with result: %d", result);
  return result;
}

static int w25q_program(block_device_t *const self, uint32_t addr,
                        const uint8_t *buf, size_t size) {
  w25qxx_t *dev = (w25qxx_t *)self;
  log_d("W25Q program called: addr=0x%08X, size=%d", addr, (int)size);
  uint32_t cur_addr = addr;
  uint32_t end_addr = addr + size;
  const uint8_t *ptr = buf;

  while (cur_addr < end_addr) {
    uint32_t remains = dev->info.page_size - (cur_addr % dev->info.page_size);
    uint32_t chunk =
        (end_addr - cur_addr < remains) ? (end_addr - cur_addr) : remains;

    if (dev->adapter->ops->write_enable(dev->adapter) != 0)
      return -1;
    if (dev->adapter->ops->program_page(dev->adapter, cur_addr, ptr, chunk) !=
        0)
      return -1;
    if (dev->adapter->ops->wait_busy(dev->adapter, 1000) != 0)
      return -1;

    cur_addr += chunk;
    ptr += chunk;
  }
  return 0;
}

static int w25q_erase(block_device_t *const self, uint32_t addr, size_t size) {
  w25qxx_t *dev = (w25qxx_t *)self;
  uint32_t cur_addr = addr;
  uint32_t end_addr = addr + size;

  while (cur_addr < end_addr) {
    if (dev->adapter->ops->write_enable(dev->adapter) != 0)
      return -1;
    if (dev->adapter->ops->erase_sector(dev->adapter, cur_addr) != 0)
      return -1;
    if (dev->adapter->ops->wait_busy(dev->adapter, 1000) != 0)
      return -1;
    cur_addr += dev->info.sector_size;
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
  if (dev)
    sys_free(W25Q_MEMSOURCE, dev);
}
