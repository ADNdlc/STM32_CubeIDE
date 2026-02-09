/*
 * stm32_one_wire_soft_driver.h
 *
 *  Created on: Jan 16, 2026
 *      Author: Antigravity
 *
 *  STM32 软件模拟 One-Wire 驱动
 */

#ifndef PLATFORM_STM32_ONE_WIRE_STM32_ONE_WIRE_SOFT_DRIVER_H_
#define PLATFORM_STM32_ONE_WIRE_STM32_ONE_WIRE_SOFT_DRIVER_H_

#include "one_wire_driver.h"
#include "stm32h7xx_hal.h"

#define ONE_WIRE_MEMSOURCE  SYS_MEM_INTERNAL

// One-Wire 软件模拟配置
typedef struct {
  GPIO_TypeDef *port; // GPIO 端口
  uint16_t pin;       // GPIO 引脚
} stm32_one_wire_config_t;

// STM32 软件模拟 One-Wire 驱动结构体
typedef struct {
  one_wire_driver_t base;         // 继承自 one_wire_driver_t 接口

  stm32_one_wire_config_t config; // 配置
  uint8_t initialized;            // 初始化标志
} stm32_one_wire_driver_t;

/**
 * @brief 创建 STM32 One-Wire 驱动实例
 * @param config One-Wire 配置参数
 * @return 驱动实例指针，失败返回 NULL
 */
one_wire_driver_t * stm32_one_wire_soft_create(const stm32_one_wire_config_t *config);
void stm32_one_wire_soft_destroy(stm32_one_wire_driver_t *drv);

#endif /* PLATFORM_STM32_ONE_WIRE_STM32_ONE_WIRE_SOFT_DRIVER_H_ */
