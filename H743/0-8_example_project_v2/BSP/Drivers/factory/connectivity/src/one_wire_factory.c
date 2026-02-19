/*
 * one_wire_factory.c
 *
 *  Created on: Jan 16, 2026
 *      Author: Antigravity
 *
 *  One-Wire 驱动工厂实现
 */
#include "one_wire_factory.h"
#include "dev_map.h"

#if (TARGET_PLATFORM == PLATFORM_STM32)
#include "one_wire/stm32_one_wire_driver.h"
#endif

#include <stddef.h>

// 存储已创建的 One-Wire 软件模拟驱动实例指针数组
static one_wire_driver_t *one_wire_drivers[ONE_WIRE_MAX_DEVICES] = {NULL};

one_wire_driver_t *one_wire_driver_get(one_wire_device_id_t id) {
  // 检查ID是否有效
  if (id >= ONE_WIRE_MAX_DEVICES) {
    return NULL;
  }
  if (one_wire_drivers[id] == NULL) { // 检查是否已初始化
    const one_wire_mapping_t *mapping = &one_wire_mappings[id];
    if (mapping->resource == NULL) {
      return NULL;
    }

#if (TARGET_PLATFORM == PLATFORM_STM32)
    // 创建驱动实例
    one_wire_drivers[id] = stm32_one_wire_soft_create(
        (stm32_one_wire_config_t *)mapping->resource);
#else
#error "one_wire_factory:未定义TARGET_PLATFORM或平台不支持"
#endif
  }

  return one_wire_drivers[id];
}
