/*
 * stm32_gpio_adapter.c
 *
 *  Created on: Nov 27, 2025
 *      Author: 12114
 */

#include "stm32_gpio_driver.h"
#include "MemPool.h"
#include <stdlib.h>

// STM32 GPIO 驱动操作实现
static void stm32_gpio_write(gpio_driver_t *self, uint8_t value) {
  stm32_gpio_driver_t *drv = (stm32_gpio_driver_t *)self;
  HAL_GPIO_WritePin(drv->config.port, drv->config.pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t stm32_gpio_read(gpio_driver_t *self) {
  stm32_gpio_driver_t *drv = (stm32_gpio_driver_t *)self;
  return (uint8_t)(HAL_GPIO_ReadPin(drv->config.port, drv->config.pin));
}

static void stm32_gpio_toggle(gpio_driver_t *self) {
  stm32_gpio_driver_t *drv = (stm32_gpio_driver_t *)self;
  HAL_GPIO_TogglePin(drv->config.port, drv->config.pin);
}

// STM32 GPIO 驱动类虚函数表
static const gpio_driver_ops_t stm32_gpio_ops = {
    .gpio_write = stm32_gpio_write,
    .gpio_read = stm32_gpio_read,
    .gpio_toggle = stm32_gpio_toggle,
};

// 创建 STM32 GPIO 驱动实例
gpio_driver_t *stm32_gpio_create(const stm32_gpio_config_t *config) {
  stm32_gpio_driver_t *drv = (stm32_gpio_driver_t *)sys_malloc(
      SYS_MEM_INTERNAL, sizeof(stm32_gpio_driver_t));
  if (drv) {
    drv->base.ops = &stm32_gpio_ops;
    drv->config = *config;
  }
  return (gpio_driver_t *)drv;
}
