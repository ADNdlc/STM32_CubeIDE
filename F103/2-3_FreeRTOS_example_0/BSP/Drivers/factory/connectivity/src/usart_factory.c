/*
 * usart_factory.c
 *
 *  Created on: Dec 1, 2025
 *      Author: 12114
 */
#include "..\inc\usart_factory.h"
#include "dev_map.h"

#if (TARGET_PLATFORM == PLATFORM_STM32)
#include "stm32_inc.h"
#endif

static usart_driver_t *usart_drivers[USART_MAX_DEVICES] = {NULL};

usart_driver_t *usart_driver_get(usart_device_id_t id) {
  if (id >= USART_MAX_DEVICES) {
    return NULL;
  }

  if (usart_drivers[id] == NULL) {  // 检查是否已初始化
    const usart_mapping_t *mapping = &usart_mappings[id]; //根据ID获取设备配置
    // 检查物理资源是否定义
    if (mapping->resource == NULL) {
      return NULL;
    }

/* 根据平台配置创建相应的USART驱动实例 */
#if (TARGET_PLATFORM == PLATFORM_STM32)
    // 在STM32平台下，resource被视为UART_HandleTypeDef指针
    usart_drivers[id] = (usart_driver_t *)stm32_usart_driver_create(
        (UART_HandleTypeDef *)mapping->resource);
#elif (TARGET_PLATFORM == PLATFORM_LINUX)
    // 假设Linux平台下，resource被视为设备路径字符串
    // usart_drivers[id] = (usart_driver_t *)linux_usart_driver_create((const
    // char *)mapping->resource);
#else
#error "usart_factory:未定义TARGET_PLATFORM或平台不支持"
#endif
  }
  return usart_drivers[id];
}
