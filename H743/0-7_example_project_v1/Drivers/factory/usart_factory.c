/*
 * usart_factory.c
 *
 *  Created on: Dec 1, 2025
 *      Author: 12114
 */

#include "usart_factory.h"
#include "factory_config.h"
#include "device_mapping.h"

#if (USART_DRIVER_PLATFORM == PLATFORM_STM32)
#include "stm32_usart_driver.h"
#endif

static usart_driver_t *usart_drivers[USART_MAX_DEVICES] = {NULL};

usart_driver_t *usart_driver_get(usart_device_id_t id) {
  if (id >= USART_MAX_DEVICES) {
    return NULL;
  }
  if (usart_drivers[id] == NULL) {
    const usart_mapping_t *mapping = &usart_mappings[id];
    
    // 根据平台配置创建相应的USART驱动实例
    #if (USART_DRIVER_PLATFORM == PLATFORM_STM32)
        usart_drivers[id] =
            (usart_driver_t *)stm32_usart_driver_create(mapping->huart);
    #else
        #error "未定义USART_DRIVER_PLATFORM或平台不支持"
    #endif
  }
  return usart_drivers[id];
}