#include "sdcard.h"
#include "elog.h"
#include "sys.h"
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "SDCard"
#define SDCARD_MEMSOURCE SYS_MEM_INTERNAL

static int sdcard_init(block_device_t *const self) {
  sdcard_t *dev = (sdcard_t *)self;
  if (!dev || !dev->adapter || !dev->adapter->ops->init) {
    return -1;
  }

  log_i("Initializing SD Card...");
  if (dev->adapter->ops->init(dev->adapter) != 0) {
    log_e("Adapter init failed");
    return -1;
  }

  sdcard_info_t card_info;
  if (dev->adapter->ops->get_info(dev->adapter, &card_info) == 0) {
    dev->info.capacity = (uint32_t)card_info.block_count * card_info.block_size;
    dev->info.block_size = card_info.block_size;
    dev->info.sector_size = card_info.block_size;
    dev->info.page_size = card_info.block_size;
    dev->info.prog_unit = card_info.block_size;
    dev->info.read_unit = card_info.block_size;
    dev->info.erase_value =
        0x00; // SD card usually returns 0x00 after erase, or doesn't support
              // random access erase like NOR

    log_i("SD Card Init OK. Size: %d MB", card_info.capacity_mb);
  } else {
    log_e("Failed to get card info");
    return -1;
  }

  return 0;
}

static int sdcard_deinit(block_device_t *const self) {
  sdcard_t *dev = (sdcard_t *)self;
  if (dev && dev->adapter) {
    // 调用重置以清理硬件状态(热插拔关键)
    if (dev->adapter->ops->reset) {
      dev->adapter->ops->reset(dev->adapter);
    }
    if (dev->adapter->ops->deinit) {
      return dev->adapter->ops->deinit(dev->adapter);
    }
  }
  return 0;
}

static int sdcard_read(block_device_t *const self, uint32_t addr, uint8_t *buf,
                       size_t size) {
  sdcard_t *dev = (sdcard_t *)self;

  if (addr % dev->info.block_size != 0 || size % dev->info.block_size != 0) {
    log_e("Read addr/size not aligned to block size %d", dev->info.block_size);
    return -1;
  }

  uint32_t start_block = addr / dev->info.block_size;
  uint32_t block_count = size / dev->info.block_size;

  return dev->adapter->ops->read_blocks(dev->adapter, start_block, buf,
                                        block_count);
}

static int sdcard_program(block_device_t *const self, uint32_t addr,
                          const uint8_t *buf, size_t size) {
  sdcard_t *dev = (sdcard_t *)self;

  if (addr % dev->info.block_size != 0 || size % dev->info.block_size != 0) {
    log_e("Program addr/size not aligned to block size %d",
          dev->info.block_size);
    return -1;
  }

  uint32_t start_block = addr / dev->info.block_size;
  uint32_t block_count = size / dev->info.block_size;

  return dev->adapter->ops->write_blocks(dev->adapter, start_block, buf,
                                         block_count);
}

static int sdcard_erase(block_device_t *const self, uint32_t addr,
                        size_t size) {
  // SD card handles its own internal erase cycles.
  // We could implement HAL_SD_EraseBlocks here if needed.
  return 0;
}

static int sdcard_get_info(block_device_t *const self, block_dev_info_t *info) {
  sdcard_t *dev = (sdcard_t *)self;
  if (info) {
    memcpy(info, &dev->info, sizeof(block_dev_info_t));
    return 0;
  }
  return -1;
}

static int sdcard_sync(block_device_t *const self) {
  sdcard_t *dev = (sdcard_t *)self;
  if (dev->adapter && dev->adapter->ops->is_ready) {
    return dev->adapter->ops->is_ready(dev->adapter);
  }
  return 0;
}

/**
 * @brief 检测SD卡是否物理存在(用于热插拔)
 * @return 1=存在, 0=不存在, -1=不支持/需重新初始化
 */
static int sdcard_is_present(block_device_t *const self) {
  sdcard_t *dev = (sdcard_t *)self;
  if (!dev || !dev->adapter) {
    return -1;
  }

  // 优先使用adapter的detect检测物理存在
  if (dev->adapter->ops->detect) {
    return dev->adapter->ops->detect(dev->adapter);
  }

  // 回退: 使用is_ready判断
  if (dev->adapter->ops->is_ready) {
    return (dev->adapter->ops->is_ready(dev->adapter) == 0) ? 1 : 0;
  }

  return -1;
}

static const block_device_ops_t sdcard_ops = {
    .init = sdcard_init,
    .deinit = sdcard_deinit,
    .read = sdcard_read,
    .program = sdcard_program,
    .erase = sdcard_erase,
    .get_info = sdcard_get_info,
    .sync = sdcard_sync,
    .is_present = sdcard_is_present,
};

block_device_t *sdcard_create(sdcard_adapter_t *adapter) {
  sdcard_t *dev = (sdcard_t *)sys_malloc(SDCARD_MEMSOURCE, sizeof(sdcard_t));
  if (dev) {
    memset(dev, 0, sizeof(sdcard_t));
    dev->parent.ops = &sdcard_ops;
    dev->adapter = adapter;
    return &dev->parent;
  }
  return NULL;
}

void sdcard_destroy(block_device_t *dev) {
  if (dev) {
    sys_free(SDCARD_MEMSOURCE, dev);
  }
}
