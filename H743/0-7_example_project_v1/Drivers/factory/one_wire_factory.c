/*
 * one_wire_factory.c
 *
 *  Created on: Jan 16, 2026
 *      Author: Antigravity
 *
 *  One-Wire 驱动工厂实现
 */

#include "one_wire_factory.h"
#include "device_mapping.h"
#include "factory_config.h"

#if (TARGET_PLATFORM == PLATFORM_STM32)
#include "one_wire/stm32_one_wire_soft_driver.h"
#endif

#include <stddef.h>

// 存储已创建的 One-Wire 软件模拟驱动实例指针数组
static one_wire_driver_t *one_wire_soft_drivers[ONE_WIRE_SOFT_MAX_DEVICES] = {
    NULL};

one_wire_driver_t *one_wire_soft_driver_get(one_wire_soft_device_id_t id) {
  // 检查ID是否有效
  if (id >= ONE_WIRE_SOFT_MAX_DEVICES) {
    return NULL;
  }

  // 如果该驱动尚未创建，则创建它
  if (one_wire_soft_drivers[id] == NULL) {
    const one_wire_soft_mapping_t *mapping = &one_wire_soft_mappings[id];

#if (TARGET_PLATFORM == PLATFORM_STM32)
    // 创建 STM32 软件模拟 One-Wire 驱动配置
    stm32_one_wire_soft_config_t config = {
        .port = mapping->port,
        .pin = mapping->pin,
    };

    // 创建驱动实例
    stm32_one_wire_soft_driver_t *drv = stm32_one_wire_soft_create(&config);
    if (drv) {
      one_wire_soft_drivers[id] = (one_wire_driver_t *)drv;
    }
#else
#error "未定义TARGET_PLATFORM或平台不支持"
#endif
  }

  return one_wire_soft_drivers[id];
}
