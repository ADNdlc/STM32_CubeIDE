/*
 * stm32_gpio_adapter.c
 *
 *  Created on: Nov 27, 2025
 *      Author: 12114
 */

#include "stm32_gpio_adapter.h"
#include <stdlib.h>

// STM32 GPIO 驱动操作实现
static void stm32_gpio_write(gpio_driver_t *self, uint8_t value) {
    stm32_gpio_driver_t *drv = (stm32_gpio_driver_t *)self;
    HAL_GPIO_WritePin(drv->port, drv->pin, value ? drv->active_level : (drv->active_level == GPIO_PIN_SET ? GPIO_PIN_RESET : GPIO_PIN_SET));
}

static void stm32_gpio_toggle(gpio_driver_t *self) {
    stm32_gpio_driver_t *drv = (stm32_gpio_driver_t *)self;
    HAL_GPIO_TogglePin(drv->port, drv->pin);
}

static uint8_t stm32_gpio_read(gpio_driver_t *self) {
    stm32_gpio_driver_t *drv = (stm32_gpio_driver_t *)self;
    return (uint8_t)(HAL_GPIO_ReadPin(drv->port, drv->pin) == drv->active_level);
}

// STM32 GPIO 驱动操作虚函数表
static const gpio_driver_ops_t stm32_gpio_ops = {
    .gpio_write = stm32_gpio_write,
    .gpio_read = stm32_gpio_read,
    .gpio_toggle = stm32_gpio_toggle
};

// 创建 STM32 GPIO 驱动实例
stm32_gpio_driver_t* stm32_gpio_create(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState active_level) {
    stm32_gpio_driver_t *drv = (stm32_gpio_driver_t*)malloc(sizeof(stm32_gpio_driver_t));
    if (drv) {
        drv->base.ops = &stm32_gpio_ops;
        drv->port = port;
        drv->pin = pin;
        drv->active_level = active_level;
    }
    return drv;
}