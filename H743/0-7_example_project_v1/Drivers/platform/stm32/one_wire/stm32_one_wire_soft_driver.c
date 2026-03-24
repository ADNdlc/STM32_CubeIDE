/*
 * stm32_one_wire_soft_driver.c
 *
 *  Created on: Jan 16, 2026
 *      Author: Antigravity
 */

#include "stm32_one_wire_soft_driver.h"
#include "SYSTEM/sys.h"
#include <stdlib.h>

// 静态函数声明
static int stm32_ow_init(one_wire_driver_t *self);
static void stm32_ow_set_mode(one_wire_driver_t *self, uint8_t mode);
static void stm32_ow_write(one_wire_driver_t *self, uint8_t data);
static uint8_t stm32_ow_read(one_wire_driver_t *self);
static void stm32_ow_release(one_wire_driver_t *self);

// 驱动操作接口实现
static const one_wire_driver_ops_t stm32_ow_ops = {
    .init = stm32_ow_init,
    .set_mode = stm32_ow_set_mode,
    .write = stm32_ow_write,
    .read = stm32_ow_read,
    .release = stm32_ow_release,
};

static int stm32_ow_init(one_wire_driver_t *self) {
  stm32_one_wire_soft_driver_t *drv = (stm32_one_wire_soft_driver_t *)self;

  // 初始化 GPIO 为输入上拉模式 (空闲状态)
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = drv->config.pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(drv->config.port, &GPIO_InitStruct);

  drv->initialized = 1;
  return 0;
}

static void stm32_ow_set_mode(one_wire_driver_t *self, uint8_t mode) {
  stm32_one_wire_soft_driver_t *drv = (stm32_one_wire_soft_driver_t *)self;
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = drv->config.pin;

  if (mode) { // 输出模式
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  } else { // 输入模式
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
  }
  HAL_GPIO_Init(drv->config.port, &GPIO_InitStruct);
}

static void stm32_ow_write(one_wire_driver_t *self, uint8_t data) {
  stm32_one_wire_soft_driver_t *drv = (stm32_one_wire_soft_driver_t *)self;
  HAL_GPIO_WritePin(drv->config.port, drv->config.pin,
                    data ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t stm32_ow_read(one_wire_driver_t *self) {
  stm32_one_wire_soft_driver_t *drv = (stm32_one_wire_soft_driver_t *)self;
  return (uint8_t)HAL_GPIO_ReadPin(drv->config.port, drv->config.pin);
}

static void stm32_ow_release(one_wire_driver_t *self) {
  stm32_ow_set_mode(self, 0); // 回到输入上拉状态
}

stm32_one_wire_soft_driver_t *
stm32_one_wire_soft_create(const stm32_one_wire_soft_config_t *config) {
  // 使用 sys_malloc 分配内存
  stm32_one_wire_soft_driver_t *drv =
      (stm32_one_wire_soft_driver_t *)sys_malloc(
          SYS_MEM_INTERNAL, sizeof(stm32_one_wire_soft_driver_t));
  if (drv) {
    drv->base.ops = &stm32_ow_ops;
    drv->config = *config;
    drv->initialized = 0;
  }
  return drv;
}

void stm32_one_wire_soft_destroy(stm32_one_wire_soft_driver_t *drv) {
  if (drv) {
    sys_free(SYS_MEM_INTERNAL, drv);
  }
}
