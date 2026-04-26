/*
 * stm32_pwm_adapter.h
 *
 *  Created on: Nov 27, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_PWM_LED_STM32_PWM_ADAPTER_H_
#define BSP_DEVICE_DRIVER_PWM_LED_STM32_PWM_ADAPTER_H_

#include <stdint.h>
#include "Connectivity\pwm_driver.h"
#include "stm32_inc.h"

typedef struct {
  TIM_HandleTypeDef *htim;
  uint32_t channel;
} stm32_pwm_config_t;

typedef struct {
  pwm_driver_t base; // 接口

  stm32_pwm_config_t config;
} stm32_pwm_driver_t;

stm32_pwm_driver_t *stm32_pwm_driver_create(stm32_pwm_config_t *config);

#endif /* BSP_DEVICE_DRIVER_PWM_LED_STM32_PWM_ADAPTER_H_ */
