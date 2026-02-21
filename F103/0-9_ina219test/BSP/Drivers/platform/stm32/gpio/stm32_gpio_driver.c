/*
 * stm32_gpio_adapter.c
 *
 *  Created on: Nov 27, 2025
 *      Author: 12114
 */

#include "stm32_gpio_driver.h"
#include <stdlib.h>
#include "MemPool.h"  // 不使用内存池就注释掉

// STM32 GPIO 驱动操作实现
static void stm32_gpio_write(gpio_driver_t *self, uint8_t value) {
  stm32_gpio_driver_t *drv = (stm32_gpio_driver_t *)self;
  HAL_GPIO_WritePin(drv->config.port, drv->config.pin,
                    value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t stm32_gpio_read(gpio_driver_t *self) {
  stm32_gpio_driver_t *drv = (stm32_gpio_driver_t *)self;
  return (uint8_t)(HAL_GPIO_ReadPin(drv->config.port, drv->config.pin));
}

static void stm32_gpio_toggle(gpio_driver_t *self) {
  stm32_gpio_driver_t *drv = (stm32_gpio_driver_t *)self;
  HAL_GPIO_TogglePin(drv->config.port, drv->config.pin);
}

static void stm32_gpio_set_mode(gpio_driver_t *self, gpio_mode_t mode) {
  stm32_gpio_driver_t *drv = (stm32_gpio_driver_t *)self;
  GPIO_InitTypeDef init = {0};

  init.Pin = drv->config.pin;
  init.Pull = GPIO_NOPULL; // 简化处理，可根据需要扩展

  switch(mode) {
    case GPIO_FloatInput:
      init.Mode = GPIO_MODE_INPUT;
      init.Pull = GPIO_NOPULL;
      break;
    case GPIO_AnalogInput:
      init.Mode = GPIO_MODE_ANALOG;
      break;
    case GPIO_PushPullOutput:
      init.Mode = GPIO_MODE_OUTPUT_PP;
      init.Speed = GPIO_SPEED_FREQ_HIGH;
      break;
    case GPIO_OpenDrainOutput:
      init.Mode = GPIO_MODE_OUTPUT_OD;
      init.Speed = GPIO_SPEED_FREQ_HIGH;
      break;
    default:
      return; // 忽略未定义的模式
  }
  HAL_GPIO_Init(drv->config.port, &init);
}

// STM32 GPIO 驱动类虚函数表
static const gpio_driver_ops_t stm32_gpio_ops = {
    .gpio_write = stm32_gpio_write,
    .gpio_read = stm32_gpio_read,
    .gpio_toggle = stm32_gpio_toggle,
    .gpio_set_mode = stm32_gpio_set_mode,
};

// 创建 STM32 GPIO 驱动实例
gpio_driver_t *stm32_gpio_create(const stm32_gpio_config_t *config) {
#ifdef USE_MEMPOOL
  stm32_gpio_driver_t *drv = (stm32_gpio_driver_t *)sys_malloc(
      SYS_MEM_INTERNAL, sizeof(stm32_gpio_driver_t));
#else
  stm32_gpio_driver_t *drv = (stm32_gpio_driver_t *)malloc(sizeof(stm32_gpio_driver_t));
#endif
  if (drv) {
    drv->base.ops = &stm32_gpio_ops;
    drv->config = *config;
  }
  return (gpio_driver_t *)drv;
}
