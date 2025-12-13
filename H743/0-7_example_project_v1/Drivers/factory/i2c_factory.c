/*
 * i2c_factory.c
 *
 *  Created on: Dec 13, 2025
 *      Author: Antigravity
 *
 *  I2C 驱动工厂实现
 */

#include "i2c_factory.h"
#include "device_mapping.h"
#include "factory_config.h"


#if (TARGET_PLATFORM == PLATFORM_STM32)
#include "stm32_i2c_soft_driver.h"
#endif

// 存储已创建的 I2C 软件模拟驱动实例指针数组
static i2c_driver_t *i2c_soft_drivers[I2C_SOFT_MAX_DEVICES] = {NULL};

i2c_driver_t *i2c_soft_driver_get(i2c_soft_device_id_t id) {
  // 检查ID是否有效
  if (id >= I2C_SOFT_MAX_DEVICES) {
    return NULL;
  }

  // 如果该驱动尚未创建，则创建它
  if (i2c_soft_drivers[id] == NULL) {
    const i2c_soft_mapping_t *mapping = &i2c_soft_mappings[id];

#if (TARGET_PLATFORM == PLATFORM_STM32)
    // 创建 STM32 软件模拟 I2C 驱动配置
    stm32_i2c_soft_config_t config = {
        .scl_port = mapping->scl_port,
        .scl_pin = mapping->scl_pin,
        .sda_port = mapping->sda_port,
        .sda_pin = mapping->sda_pin,
        .delay_us = mapping->delay_us,
    };

    // 创建驱动实例
    stm32_i2c_soft_driver_t *drv = stm32_i2c_soft_create(&config);
    if (drv) {
      i2c_soft_drivers[id] = (i2c_driver_t *)drv;
    }
#else
#error "未定义TARGET_PLATFORM或平台不支持"
#endif
  }

  return i2c_soft_drivers[id];
}
