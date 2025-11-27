/*
 * stm32_pwm_adapter.h
 *
 *  Created on: Nov 27, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_PWM_LED_STM32_PWM_ADAPTER_H_
#define BSP_DEVICE_DRIVER_PWM_LED_STM32_PWM_ADAPTER_H_

#include "pwm_driver.h"

#ifdef USE_HAL_DRIVER
#include "stm32h7xx_hal.h" // 根据实际芯片系列调整
#else
// Mock for non-HAL environment or generic include
typedef struct {
  void *Instance;
} TIM_HandleTypeDef;
#endif

// 基础操作为基类，平台继承并重写自己的特有操作，设备驱动依赖基类
typedef struct {
  pwm_driver_t base; // 继承基类

  // STM32 特有数据
  TIM_HandleTypeDef *htim;
  uint32_t channel;
} stm32_pwm_driver_t;

// 构造函数
stm32_pwm_driver_t *stm32_pwm_driver_create(TIM_HandleTypeDef *htim,
                                            uint32_t channel);

#endif /* BSP_DEVICE_DRIVER_PWM_LED_STM32_PWM_ADAPTER_H_ */
