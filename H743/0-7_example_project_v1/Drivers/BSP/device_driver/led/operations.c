/*
 * operations.c
 *
 *  Created on: Nov 26, 2025
 *      Author: 12114
 */
#include "operations.h"
#include "led.h"

/* ----- HAL库依赖的具体实现 ----- */
#ifdef _USE_HAL_DRIVER
#include "main.h"

// HAL GPIO操作的具体实现
static void hal_write_pin(void* port, uint16_t pin, uint8_t value) {
    HAL_GPIO_WritePin((GPIO_TypeDef*)port, pin, (GPIO_PinState)value);
}

static void hal_toggle_pin(void* port, uint16_t pin) {
    HAL_GPIO_TogglePin((GPIO_TypeDef*)port, pin);
}

static uint8_t hal_read_pin(void* port, uint16_t pin) {
    return (uint8_t)HAL_GPIO_ReadPin((GPIO_TypeDef*)port, pin);
}

// HAL GPIO操作函数集合
static const gpio_operations_t hal_gpio_ops = {
    .write_pin = hal_write_pin,
    .toggle_pin = hal_toggle_pin,
    .read_pin = hal_read_pin
};

#endif

/* ----- 标准库库依赖的具体实现 ----- */

//...
