/*
 * stm32_pwm_adapter.c
 *
 *  Created on: Nov 27, 2025
 *      Author: 12114
 */

#include "stm32_pwm.h"
#include <stdlib.h>

#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_bus.h"

uint32_t Get_TIM_Freq_LL(TIM_TypeDef *TIMx)
{
    LL_RCC_ClocksTypeDef rcc_clocks;
    uint32_t tim_input_clk = 0;
    
    // 1. 获取系统各总线时钟频率 (HCLK, PCLK1, PCLK2)
    // 这一步 LL 库会自动读取寄存器并计算当前各总线频率
    LL_RCC_GetSystemClocksFreq(&rcc_clocks);

    // 2. 根据定时器所在的 APB 总线，利用 LL 宏计算输入时钟
    // 注意：你需要知道你的 TIMx 挂载在哪个 APB 上
    if (TIMx == TIM1 || TIMx == TIM8 || TIMx == TIM9 || TIMx == TIM10 || TIMx == TIM11) 
    {
        // === APB2 定时器 ===
        // 宏会自动处理 "x1" 还是 "x2" 的逻辑
        tim_input_clk = __LL_RCC_CALC_APB2_TIM_FREQ(rcc_clocks.HCLK_Frequency, 
                                                    LL_RCC_GetAPB2Prescaler());
    }
    else 
    {
        // === APB1 定时器 (TIM2, TIM3, TIM4...) ===
        tim_input_clk = __LL_RCC_CALC_APB1_TIM_FREQ(rcc_clocks.HCLK_Frequency, 
                                                    LL_RCC_GetAPB1Prescaler());
    }

    // 3. 获取定时器内部的预分频系数 (PSC)
    uint32_t psc = LL_TIM_GetPrescaler(TIMx);

    // 4. 计算最终计数频率
    return tim_input_clk / (psc + 1);
}


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

static void _stm32_pwm_set_freq(pwm_driver_t *base, uint32_t freq) {
  stm32_pwm_driver_t *self = (stm32_pwm_driver_t *)base;
#ifdef USE_HAL_DRIVER
  HAL_TIM_Base_Start_IT(self->htim);
  HAL_TIM_Base_SetFrequency(self->htim, freq);
}

static uint32_t _stm32_pwm_get_freq(pwm_driver_t *base) {
  stm32_pwm_driver_t *self = (stm32_pwm_driver_t *)base;
#ifdef USE_HAL_DRIVER
  return HAL_TIM_Base_GetFrequency(self->htim);
#endif
}

static const pwm_driver_ops_t stm32_pwm_ops = {
    .start = _stm32_pwm_start,
    .stop = _stm32_pwm_stop,
    .set_duty = _stm32_pwm_set_duty,
    .set_freq = _stm32_pwm_set_freq,
    .get_freq = _stm32_pwm_get_freq
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
