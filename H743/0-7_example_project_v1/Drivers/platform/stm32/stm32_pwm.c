/*
 * stm32_pwm_adapter.c
 *
 *  Created on: Nov 27, 2025
 *      Author: 12114
 */

#include "stm32_pwm_adapter.h"
#include <stdlib.h>

// STM32 HAL 实现
static void _stm32_pwm_start(pwm_driver_t *base) {
  stm32_pwm_driver_t *self = (stm32_pwm_driver_t *)base;
#ifdef USE_HAL_DRIVER
  HAL_TIM_PWM_Start(self->htim, self->channel);
#endif
}

static void _stm32_pwm_stop(pwm_driver_t *base) {
  stm32_pwm_driver_t *self = (stm32_pwm_driver_t *)base;
#ifdef USE_HAL_DRIVER
  HAL_TIM_PWM_Stop(self->htim, self->channel);
#endif
}

static void _stm32_pwm_set_duty(pwm_driver_t *base, uint32_t duty) {
  stm32_pwm_driver_t *self = (stm32_pwm_driver_t *)base;
#ifdef USE_HAL_DRIVER
  __HAL_TIM_SET_COMPARE(self->htim, self->channel, duty);
#endif
}

static const pwm_driver_ops_t stm32_pwm_ops = {
    .start = _stm32_pwm_start,
    .stop = _stm32_pwm_stop,
    .set_duty = _stm32_pwm_set_duty,
};

stm32_pwm_driver_t *stm32_pwm_driver_create(TIM_HandleTypeDef *htim,
                                            uint32_t channel) {
  stm32_pwm_driver_t *self =
      (stm32_pwm_driver_t *)malloc(sizeof(stm32_pwm_driver_t));
  if (self) {
    self->base.ops = &stm32_pwm_ops;
    self->htim = htim;
    self->channel = channel;
  }
  return self;
}
