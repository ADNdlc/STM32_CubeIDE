/*
 * i2c_factory.c
 *
 *  Created on: Feb 11, 2026
 *      Author: Antigravity
 *
 *  I2C 驱动工厂实现
 */

#include "..\inc\i2c_factory.h"
#include "dev_map.h"
#include <stddef.h>

#if (TARGET_PLATFORM == PLATFORM_STM32)
#include "stm32_inc.h"

#endif

// 存储已创建的 I2C 驱动实例指针数组
static i2c_driver_t *i2c_drivers[I2C_MAX_DEVICES] = {NULL};

i2c_driver_t *i2c_driver_get(i2c_device_id_t id) {
  if (id >= I2C_MAX_DEVICES) {
    return NULL;
  }

  if (i2c_drivers[id] == NULL) {
    const i2c_mapping_t *mapping = &i2c_mappings[id];
    if (mapping->resource == NULL) {
      return NULL;
    }

#if (TARGET_PLATFORM == PLATFORM_STM32)
    // 在 STM32 平台下，resource 被视为 stm32_i2c_config_t 指针
    i2c_drivers[id] =
        STM32_I2C_DRIVER_CREATE((stm32_i2c_config_t *)mapping->resource);
#else
#error "i2c_factory: 未定义 TARGET_PLATFORM 或平台不支持"
#endif
  }

  return i2c_drivers[id];
}
