/*
 * flash_factory.c
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  Flash 驱动工厂模式实现
 */

#include "flash_factory.h"
#include "flash_default_strategy.h"
#include "w25q256_driver.h"
#include <string.h>


// ============== 工厂方法实现 ==============

flash_driver_t *flash_factory_create(const flash_factory_config_t *config) {
  if (config == NULL || config->spi_adapter == NULL) {
    return NULL;
  }

  flash_driver_t *driver = NULL;
  flash_dependencies_t *deps = config->deps;

  // 如果没有提供依赖，使用默认
  if (deps == NULL) {
    deps = flash_dependencies_create_default();
  }

  // 设置 SPI 适配器
  if (deps) {
    deps->spi_adapter = config->spi_adapter;
  }

  // 根据芯片类型创建驱动
  switch (config->chip_type) {
  case FLASH_CHIP_W25Q256:
    driver = w25q256_driver_create(config->spi_adapter, deps);
    break;

  case FLASH_CHIP_W25Q128:
  case FLASH_CHIP_W25Q64:
  case FLASH_CHIP_W25Q32:
  case FLASH_CHIP_W25Q16:
    // TODO: 添加其他型号的驱动
    // 暂时使用 W25Q256 驱动，它会自动识别芯片
    driver = w25q256_driver_create(config->spi_adapter, deps);
    break;

  default:
    return NULL;
  }

  // 设置设备名称
  if (driver && config->device_name) {
    strncpy(driver->name, config->device_name, sizeof(driver->name) - 1);
  }

  return driver;
}

void flash_factory_destroy(flash_driver_t *driver) {
  if (driver == NULL) {
    return;
  }

  // 根据驱动类型调用对应的销毁函数
  // 目前只有 W25Q256
  w25q256_driver_destroy(driver);
}

// ============== 预配置工厂方法 ==============

flash_driver_t *flash_factory_create_w25q256_qspi(spi_adapter_t *adapter,
                                                  flash_dependencies_t *deps) {
  flash_factory_config_t config = {
      .chip_type = FLASH_CHIP_W25Q256,
      .spi_adapter = adapter,
      .deps = deps,
      .device_name = "W25Q256_QSPI",
  };

  return flash_factory_create(&config);
}

flash_driver_t *flash_factory_create_w25q256_spi(spi_adapter_t *adapter,
                                                 flash_dependencies_t *deps) {
  flash_factory_config_t config = {
      .chip_type = FLASH_CHIP_W25Q256,
      .spi_adapter = adapter,
      .deps = deps,
      .device_name = "W25Q256_SPI",
  };

  return flash_factory_create(&config);
}

flash_driver_t *flash_factory_create_w25q128(spi_adapter_t *adapter,
                                             flash_dependencies_t *deps) {
  flash_factory_config_t config = {
      .chip_type = FLASH_CHIP_W25Q128,
      .spi_adapter = adapter,
      .deps = deps,
      .device_name = "W25Q128",
  };

  return flash_factory_create(&config);
}

// ============== 完整系统创建 ==============

// 静态 Handler 实例
static flash_handler_t g_flash_handler;
static uint8_t g_handler_created = 0;

flash_handler_t *
flash_factory_create_system(const flash_factory_config_t *config) {
  if (config == NULL) {
    return NULL;
  }

  if (g_handler_created) {
    return &g_flash_handler;
  }

  // 获取依赖
  flash_dependencies_t *deps = config->deps;
  if (deps == NULL) {
    deps = flash_dependencies_create_default();
  }

  // 初始化 Handler
  flash_error_t err = flash_handler_init(&g_flash_handler, deps);
  if (err != FLASH_OK) {
    return NULL;
  }

  // 创建驱动
  flash_driver_t *driver = flash_factory_create(config);
  if (driver == NULL) {
    flash_handler_deinit(&g_flash_handler);
    return NULL;
  }

  // 初始化驱动
  err = FLASH_INIT(driver);
  if (err != FLASH_OK) {
    flash_factory_destroy(driver);
    flash_handler_deinit(&g_flash_handler);
    return NULL;
  }

  // 注册驱动到 Handler
  const char *name = config->device_name ? config->device_name : "FLASH0";
  err = flash_handler_register(&g_flash_handler, name, driver);
  if (err != FLASH_OK) {
    FLASH_DEINIT(driver);
    flash_factory_destroy(driver);
    flash_handler_deinit(&g_flash_handler);
    return NULL;
  }

  // 设置默认策略
  flash_handler_set_strategy(&g_flash_handler, flash_default_strategy_create());

  g_handler_created = 1;

  return &g_flash_handler;
}

void flash_factory_destroy_system(flash_handler_t *handler) {
  if (handler == &g_flash_handler && g_handler_created) {
    flash_handler_deinit(handler);
    g_handler_created = 0;
  }
}
