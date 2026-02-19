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

// One-Wire 软件模拟配置
typedef struct {
  GPIO_TypeDef *port; // GPIO 端口
  uint16_t pin;       // GPIO 引脚
} stm32_gpio_config_t;

// STM32 GPIO 驱动结构体
typedef struct {
    gpio_driver_t base;  // 继承自 gpio_driver_t 基类
    //stm32特有 GPIO 端口和引脚 类型
    stm32_gpio_config_t config;
} stm32_gpio_driver_t;

// 创建 STM32 GPIO 驱动实例
gpio_driver_t* stm32_gpio_create(const stm32_gpio_config_t *config);

#endif /* BSP_DEVICE_DRIVER_LED_STM32_GPIO_ADAPTER_H_ */
