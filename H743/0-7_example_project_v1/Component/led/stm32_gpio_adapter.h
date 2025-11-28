/*
 * stm32_gpio_adapter.h
 *
 *  Created on: Nov 27, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_LED_STM32_GPIO_ADAPTER_H_
#define BSP_DEVICE_DRIVER_LED_STM32_GPIO_ADAPTER_H_

#include "gpio_driver.h"
#include "stm32h7xx_hal.h"

// STM32 GPIO 驱动结构体
typedef struct {
    gpio_driver_t base;  // 继承自 gpio_driver_t 基类
    GPIO_TypeDef* port;
    uint16_t pin;
    GPIO_PinState active_level;
} stm32_gpio_driver_t;

// 创建 STM32 GPIO 驱动实例
stm32_gpio_driver_t* stm32_gpio_create(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState active_level);

#endif /* BSP_DEVICE_DRIVER_LED_STM32_GPIO_ADAPTER_H_ */