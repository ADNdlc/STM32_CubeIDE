/*
 * gpio_factory.c
 *
 *  Created on: Nov 28, 2025
 *      Author: 12114
 */

#include "gpio_factory.h"
#include "stm32_gpio.h"
#include "device_mapping.h"

// 存储已创建的GPIO驱动实例指针数组
static gpio_driver_t* gpio_drivers[GPIO_MAX_DEVICES] = {NULL};

gpio_driver_t* gpio_driver_get(gpio_device_id_t id) {
    // 检查ID是否有效
    if (id >= GPIO_MAX_DEVICES) {
        return NULL;
    }
    // 如果该驱动尚未创建，则创建它
    if (gpio_drivers[id] == NULL) {
        const gpio_mapping_t* mapping = &gpio_mappings[id];
        // 创建具体的STM32 GPIO驱动实例
        gpio_drivers[id] = (gpio_driver_t*)stm32_gpio_create(mapping->port, mapping->pin);
    }
    // 返回驱动实例
    return gpio_drivers[id];
}