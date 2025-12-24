/*
 * flash_handler.c
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  Flash Handler 实现
 */

#include "flash_handler.h"
#include <string.h>

// ============== 初始化接口 ==============

flash_error_t flash_handler_init(flash_handler_t *handler,
                                 flash_dependencies_t *deps) {
  if (handler == NULL) {
    return FLASH_ERR_PARAM;
  }

  // 清零结构体
  memset(handler, 0, sizeof(flash_handler_t));

  // 设置依赖
  if (deps != NULL) {
    handler->deps = deps;
  } else {
    // 尝试获取默认依赖
    handler->deps = flash_dependencies_create_default();
  }

  handler->initialized = 1;

  if (handler->deps && handler->deps->log_info) {
    handler->deps->log_info("FLASH", "Handler initialized");
  }

  return FLASH_OK;
}

flash_error_t flash_handler_deinit(flash_handler_t *handler) {
  if (handler == NULL) {
    return FLASH_ERR_PARAM;
  }

  // 注销所有设备
  for (int i = 0; i < MAX_FLASH_DEVICES; i++) {
    if (handler->devices[i].in_use && handler->devices[i].driver) {
      FLASH_DEINIT(handler->devices[i].driver);
      handler->devices[i].in_use = 0;
    }
  }

  handler->device_count = 0;
  handler->initialized = 0;

  return FLASH_OK;
}

// ============== 注册器模式接口 ==============

flash_error_t flash_handler_register(flash_handler_t *handler, const char *name,
                                     flash_driver_t *driver) {
  if (handler == NULL || name == NULL || driver == NULL) {
    return FLASH_ERR_PARAM;
  }

  if (!handler->initialized) {
    return FLASH_ERR_NOT_INIT;
  }

  // 检查是否已满
  if (handler->device_count >= MAX_FLASH_DEVICES) {
    FLASH_LOG_E(handler->deps, "Device registry full");
    return FLASH_ERR_PARAM;
  }

  // 检查名称是否已存在
  if (flash_handler_find(handler, name) != NULL) {
    FLASH_LOG_E(handler->deps, "Device '%s' already registered", name);
    return FLASH_ERR_PARAM;
  }

  // 查找空闲槽位
  for (int i = 0; i < MAX_FLASH_DEVICES; i++) {
    if (!handler->devices[i].in_use) {
      handler->devices[i].driver = driver;
      strncpy(handler->devices[i].name, name, FLASH_DEVICE_NAME_MAX - 1);
      handler->devices[i].name[FLASH_DEVICE_NAME_MAX - 1] = '\0';
      handler->devices[i].in_use = 1;
      handler->device_count++;

      // 同时设置驱动的名称
      strncpy(driver->name, name, sizeof(driver->name) - 1);

      FLASH_LOG_I(handler->deps, "Device '%s' registered", name);
      return FLASH_OK;
    }
  }

  return FLASH_ERR_PARAM;
}

flash_error_t flash_handler_unregister(flash_handler_t *handler,
                                       const char *name) {
  if (handler == NULL || name == NULL) {
    return FLASH_ERR_PARAM;
  }

  for (int i = 0; i < MAX_FLASH_DEVICES; i++) {
    if (handler->devices[i].in_use &&
        strcmp(handler->devices[i].name, name) == 0) {

      // 去初始化驱动
      if (handler->devices[i].driver) {
        FLASH_DEINIT(handler->devices[i].driver);
      }

      handler->devices[i].in_use = 0;
      handler->devices[i].driver = NULL;
      handler->device_count--;

      FLASH_LOG_I(handler->deps, "Device '%s' unregistered", name);
      return FLASH_OK;
    }
  }

  FLASH_LOG_W(handler->deps, "Device '%s' not found", name);
  return FLASH_ERR_PARAM;
}

flash_driver_t *flash_handler_find(flash_handler_t *handler, const char *name) {
  if (handler == NULL || name == NULL) {
    return NULL;
  }

  for (int i = 0; i < MAX_FLASH_DEVICES; i++) {
    if (handler->devices[i].in_use &&
        strcmp(handler->devices[i].name, name) == 0) {
      return handler->devices[i].driver;
    }
  }

  return NULL;
}

uint8_t flash_handler_get_device_count(flash_handler_t *handler) {
  if (handler == NULL) {
    return 0;
  }
  return handler->device_count;
}

flash_driver_t *flash_handler_get_device_by_index(flash_handler_t *handler,
                                                  uint8_t index) {
  if (handler == NULL) {
    return NULL;
  }

  uint8_t count = 0;
  for (int i = 0; i < MAX_FLASH_DEVICES; i++) {
    if (handler->devices[i].in_use) {
      if (count == index) {
        return handler->devices[i].driver;
      }
      count++;
    }
  }

  return NULL;
}

// ============== 策略模式接口 ==============

void flash_handler_set_strategy(flash_handler_t *handler,
                                flash_strategy_t *strategy) {
  if (handler == NULL) {
    return;
  }
  handler->strategy = strategy;

  if (strategy) {
    FLASH_LOG_I(handler->deps, "Strategy set: %s",
                strategy->name ? strategy->name : "unnamed");
  }
}

flash_strategy_t *flash_handler_get_strategy(flash_handler_t *handler) {
  if (handler == NULL) {
    return NULL;
  }
  return handler->strategy;
}

// ============== 内部辅助函数 ==============

// 获取设备（如果device_name为NULL，使用第一个设备）
static flash_driver_t *get_device(flash_handler_t *handler,
                                  const char *device_name) {
  if (device_name != NULL) {
    return flash_handler_find(handler, device_name);
  }

  // 返回第一个可用设备
  return flash_handler_get_device_by_index(handler, 0);
}

// ============== 统一操作接口 ==============

flash_error_t flash_handler_read(flash_handler_t *handler,
                                 const char *device_name, uint32_t addr,
                                 uint8_t *buf, uint32_t len) {
  if (handler == NULL || buf == NULL || len == 0) {
    return FLASH_ERR_PARAM;
  }

  // 如果有策略，优先使用策略
  if (handler->strategy != NULL) {
    return STRATEGY_READ(handler->strategy, addr, buf, len);
  }

  // 否则直接调用驱动
  flash_driver_t *driver = get_device(handler, device_name);
  if (driver == NULL) {
    FLASH_LOG_E(handler->deps, "Device not found");
    return FLASH_ERR_PARAM;
  }

  return FLASH_READ(driver, addr, buf, len);
}

flash_error_t flash_handler_write(flash_handler_t *handler,
                                  const char *device_name, uint32_t addr,
                                  const uint8_t *buf, uint32_t len) {
  if (handler == NULL || buf == NULL || len == 0) {
    return FLASH_ERR_PARAM;
  }

  // 如果有策略，优先使用策略
  if (handler->strategy != NULL) {
    return STRATEGY_WRITE(handler->strategy, addr, buf, len);
  }

  // 否则直接调用驱动
  flash_driver_t *driver = get_device(handler, device_name);
  if (driver == NULL) {
    FLASH_LOG_E(handler->deps, "Device not found");
    return FLASH_ERR_PARAM;
  }

  return FLASH_WRITE(driver, addr, buf, len);
}

flash_error_t flash_handler_erase_sector(flash_handler_t *handler,
                                         const char *device_name,
                                         uint32_t addr) {
  if (handler == NULL) {
    return FLASH_ERR_PARAM;
  }

  flash_driver_t *driver = get_device(handler, device_name);
  if (driver == NULL) {
    return FLASH_ERR_PARAM;
  }

  return FLASH_ERASE_SECTOR(driver, addr);
}

flash_error_t flash_handler_erase_block(flash_handler_t *handler,
                                        const char *device_name,
                                        uint32_t addr) {
  if (handler == NULL) {
    return FLASH_ERR_PARAM;
  }

  flash_driver_t *driver = get_device(handler, device_name);
  if (driver == NULL) {
    return FLASH_ERR_PARAM;
  }

  return FLASH_ERASE_BLOCK(driver, addr);
}

flash_error_t flash_handler_erase(flash_handler_t *handler,
                                  const char *device_name, uint32_t addr,
                                  uint32_t len) {
  if (handler == NULL || len == 0) {
    return FLASH_ERR_PARAM;
  }

  // 如果有策略，使用策略
  if (handler->strategy != NULL) {
    return STRATEGY_ERASE(handler->strategy, addr, len);
  }

  // 否则按扇区擦除
  flash_driver_t *driver = get_device(handler, device_name);
  if (driver == NULL) {
    return FLASH_ERR_PARAM;
  }

  const flash_info_t *info = FLASH_GET_INFO(driver);
  if (info == NULL) {
    return FLASH_ERR_PARAM;
  }

  uint32_t sector_size = info->sector_size;
  uint32_t start_sector = addr / sector_size;
  uint32_t end_sector = (addr + len - 1) / sector_size;

  for (uint32_t i = start_sector; i <= end_sector; i++) {
    flash_error_t err = FLASH_ERASE_SECTOR(driver, i * sector_size);
    if (err != FLASH_OK) {
      return err;
    }
  }

  return FLASH_OK;
}

// ============== 辅助接口 ==============

flash_dependencies_t *flash_handler_get_deps(flash_handler_t *handler) {
  if (handler == NULL) {
    return NULL;
  }
  return handler->deps;
}

void flash_handler_print_devices(flash_handler_t *handler) {
  if (handler == NULL || handler->deps == NULL) {
    return;
  }

  FLASH_LOG_I(handler->deps, "=== Registered Flash Devices ===");
  FLASH_LOG_I(handler->deps, "Total: %d", handler->device_count);

  for (int i = 0; i < MAX_FLASH_DEVICES; i++) {
    if (handler->devices[i].in_use) {
      flash_driver_t *drv = handler->devices[i].driver;
      FLASH_LOG_I(handler->deps, "[%d] %s - ID:0x%06X, Size:%luMB", i,
                  handler->devices[i].name, drv->info.jedec_id,
                  drv->info.capacity / (1024 * 1024));
    }
  }
}
