/*
 * gpio_factory.c
 *
 *  Created on: Nov 28, 2025
 *      Author: 12114
 */

#include "gpio_factory.h"
#include "dev_map.h"

#if (TARGET_PLATFORM == PLATFORM_STM32)
#include "gpio/stm32_gpio_driver.h"
#endif

// 存储已创建的GPIO驱动实例指针数组
static gpio_driver_t *gpio_drivers[GPIO_MAX_DEVICES] = {NULL};

gpio_driver_t *gpio_driver_get(gpio_device_id_t id) {
  // 检查ID是否有效
  if (id >= GPIO_MAX_DEVICES) {
    return NULL;
  }
  // 如果该驱动尚未创建，则创建它
  if (gpio_drivers[id] == NULL) {
    const gpio_mapping_t *mapping = &gpio_mappings[id];

// 根据平台配置创建相应的GPIO驱动实例
#if (TARGET_PLATFORM == PLATFORM_STM32)
    // 创建具体的STM32 GPIO驱动实例
    gpio_drivers[id] =
        (gpio_driver_t *)stm32_gpio_create((stm32_gpio_config_t *)mapping->resource);
#else
#error "未定义GPIO_DRIVER_PLATFORM或平台不支持"
#endif
  }
  // 返回驱动实例
  return gpio_drivers[id];
}
