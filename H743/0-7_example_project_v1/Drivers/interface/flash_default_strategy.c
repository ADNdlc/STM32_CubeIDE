/*
 * flash_default_strategy.c
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  默认存储策略实现（直通式读写）
 */

#include "flash_default_strategy.h"
#include <string.h>

// 策略静态实例
static flash_strategy_t g_default_strategy;
static uint8_t g_strategy_initialized = 0;

// ============== 策略操作实现 ==============

static flash_error_t default_init(flash_strategy_t *self,
                                  flash_driver_t *driver) {
  if (self == NULL || driver == NULL) {
    return FLASH_ERR_PARAM;
  }

  self->driver = driver;
  return FLASH_OK;
}

static flash_error_t default_deinit(flash_strategy_t *self) {
  if (self == NULL) {
    return FLASH_ERR_PARAM;
  }

  self->driver = NULL;
  return FLASH_OK;
}

static flash_error_t default_read(flash_strategy_t *self, uint32_t addr,
                                  uint8_t *buf, uint32_t len) {
  if (self == NULL || self->driver == NULL) {
    return FLASH_ERR_NOT_INIT;
  }

  return FLASH_READ(self->driver, addr, buf, len);
}

static flash_error_t default_write(flash_strategy_t *self, uint32_t addr,
                                   const uint8_t *buf, uint32_t len) {
  if (self == NULL || self->driver == NULL) {
    return FLASH_ERR_NOT_INIT;
  }

  // 直接写入，不处理跨页
  // 注意：调用者需确保已擦除目标区域
  return FLASH_WRITE(self->driver, addr, buf, len);
}

static flash_error_t default_erase(flash_strategy_t *self, uint32_t addr,
                                   uint32_t len) {
  if (self == NULL || self->driver == NULL) {
    return FLASH_ERR_NOT_INIT;
  }

  const flash_info_t *info = FLASH_GET_INFO(self->driver);
  if (info == NULL) {
    return FLASH_ERR_PARAM;
  }

  // 按扇区擦除
  uint32_t sector_size = info->sector_size;
  uint32_t start_sector = addr / sector_size;
  uint32_t end_sector = (addr + len - 1) / sector_size;

  for (uint32_t i = start_sector; i <= end_sector; i++) {
    flash_error_t err = FLASH_ERASE_SECTOR(self->driver, i * sector_size);
    if (err != FLASH_OK) {
      return err;
    }
  }

  return FLASH_OK;
}

static flash_error_t default_sync(flash_strategy_t *self) {
  // 直通策略无缓存，无需同步
  (void)self;
  return FLASH_OK;
}

static uint32_t default_get_free_space(flash_strategy_t *self) {
  if (self == NULL || self->driver == NULL) {
    return 0;
  }

  // 直通策略无法追踪使用情况，返回总容量
  return self->driver->info.capacity;
}

static uint32_t default_get_used_space(flash_strategy_t *self) {
  // 直通策略无法追踪使用情况
  (void)self;
  return 0;
}

// 操作表
static const flash_strategy_ops_t g_default_ops = {
    .init = default_init,
    .deinit = default_deinit,
    .read = default_read,
    .write = default_write,
    .erase = default_erase,
    .sync = default_sync,
    .get_free_space = default_get_free_space,
    .get_used_space = default_get_used_space,
};

// ============== 公共接口 ==============

flash_strategy_t *flash_default_strategy_create(void) {
  if (!g_strategy_initialized) {
    memset(&g_default_strategy, 0, sizeof(g_default_strategy));
    g_default_strategy.ops = &g_default_ops;
    g_default_strategy.type = FLASH_STRATEGY_RAW;
    g_default_strategy.name = "RAW";
    g_strategy_initialized = 1;
  }

  return &g_default_strategy;
}

void flash_default_strategy_destroy(flash_strategy_t *strategy) {
  if (strategy == &g_default_strategy) {
    strategy->driver = NULL;
    // 静态实例不释放，保持可重用
  }
}
