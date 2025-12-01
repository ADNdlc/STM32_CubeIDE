/*
 * stm32_pwm_adapter.c
 *
 *  Created on: Nov 27, 2025
 *      Author: 12114
 */

#include "stm32_pwm_driver.h"
#include <stdlib.h>

// 确保包含了对应的 LL 库
#include "stm32h7xx_ll_tim.h"
#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_bus.h"

/*
 * 获取 APB1或 APB2总线 TIM 时钟源频率
 */
static uint32_t Get_H7_TIM_Input_Clock(TIM_HandleTypeDef *htim)
{
  uint32_t pclk = 0;
  uint32_t tim_ker_clk = 0;
  uint32_t prescaler_bits = 0;
  uint32_t rcc_cfgr = RCC->CFGR;
  uint32_t rcc_d2cfgr = RCC->D2CFGR;

  // 1. 区分 APB1/APB2 (此处逻辑假设仅处理 D2 域定时器)
  if (htim->Instance == TIM1 || htim->Instance == TIM8 ||
      htim->Instance == TIM15 || htim->Instance == TIM16 || htim->Instance == TIM17)
  {
    pclk = HAL_RCC_GetPCLK2Freq();
    prescaler_bits = (rcc_d2cfgr & RCC_D2CFGR_D2PPRE2) >> RCC_D2CFGR_D2PPRE2_Pos;
  }
  else // TIM2-7, 12-14
  {
    pclk = HAL_RCC_GetPCLK1Freq();
    prescaler_bits = (rcc_d2cfgr & RCC_D2CFGR_D2PPRE1) >> RCC_D2CFGR_D2PPRE1_Pos;
  }

  // 2. 计算倍频
  if ((prescaler_bits & 0x04) == 0) // Div1
  {
    tim_ker_clk = pclk;
  }
  else
  {
    if ((rcc_cfgr & RCC_CFGR_TIMPRE) && (prescaler_bits > 4)) // Div > 2 & TIMPRE=1
    {
      tim_ker_clk = pclk * 4;
    }
    else
    {
      tim_ker_clk = pclk * 2;
    }
  }

  return tim_ker_clk;
}

static void _stm32_pwm_start(pwm_driver_t *base)
{
  stm32_pwm_driver_t *self = (stm32_pwm_driver_t *)base;
  HAL_TIM_PWM_Start(self->htim, self->channel);
}

static void _stm32_pwm_stop(pwm_driver_t *base)
{
  stm32_pwm_driver_t *self = (stm32_pwm_driver_t *)base;
  HAL_TIM_PWM_Stop(self->htim, self->channel);
}

static void _stm32_pwm_set_duty(pwm_driver_t *base, uint32_t duty)
{
  stm32_pwm_driver_t *self = (stm32_pwm_driver_t *)base;
  // 直接设置 CCR 寄存器
  __HAL_TIM_SET_COMPARE(self->htim, self->channel, duty);
}

static uint32_t _stm32_pwm_get_duty_max(pwm_driver_t *base)
{
  stm32_pwm_driver_t *self = (stm32_pwm_driver_t *)base;
  return __HAL_TIM_GET_AUTORELOAD(self->htim);
}

// 动态计算 PSC 和 ARR
static void _stm32_pwm_set_freq(pwm_driver_t *base, uint32_t freq)
{
  stm32_pwm_driver_t *self = (stm32_pwm_driver_t *)base;
  if (freq == 0)
    return;

  // 1. 获取输入源时钟 (例如 200MHz)
  uint32_t tim_ker_clk = Get_H7_TIM_Input_Clock(self->htim);

  uint32_t psc = 0;
  uint32_t arr = 0;

  // 2. 动态计算 PSC 和 ARR
  // 目标： Timer_Freq = Tim_Ker_Clk / ((PSC+1) * (ARR+1))
  // 为了高分辨率，我们希望 ARR 尽可能大，PSC 尽可能小

  // 判断定时器位数 (TIM2/TIM5 是32位，其他通常16位),具体查看型号对应手册
  // 这里简单粗暴按16位处理兼容性最好，或者是通过 IS_TIM_32B_COUNTER 宏判断
  // 假设最大 ARR 为 65535 (16-bit) 以保证兼容性
  const uint32_t MAX_ARR = 0xFFFF;

  // 计算总的分频因子
  // cycles = Tim_Ker_Clk / freq
  uint32_t cycles = tim_ker_clk / freq;

  if (cycles <= MAX_ARR)
  {
    // 如果不需要预分频就能装下
    psc = 0;
    arr = cycles - 1;
  }
  else
  {
    // 需要预分频
    // psc = (cycles / 65536)
    psc = (cycles / (MAX_ARR + 1));
    // 重新计算 arr
    arr = (tim_ker_clk / ((psc + 1) * freq)) - 1;
  }

  // 3. 写入寄存器
  __HAL_TIM_SET_PRESCALER(self->htim, psc);
  __HAL_TIM_SET_AUTORELOAD(self->htim, arr);

  // 4. 更新寄存器（产生 Update Event 载入新的 PSC 和 ARR）
  // 没有这一步，PSC 要等到下一次溢出才生效
  // 注意：这会清空计数器，可能会导致当前周期的波形截断
  HAL_TIM_GenerateEvent(self->htim, TIM_EVENTSOURCE_UPDATE);

}

static uint32_t _stm32_pwm_get_freq(pwm_driver_t *base)
{
  stm32_pwm_driver_t *self = (stm32_pwm_driver_t *)base;
  // 获取输入源时钟
  uint32_t tim_ker_clk = Get_H7_TIM_Input_Clock(self->htim);

  // 获取当前寄存器值
  uint32_t psc = self->htim->Instance->PSC;
  uint32_t arr = __HAL_TIM_GET_AUTORELOAD(self->htim);

  // 计算: Freq = Clk / ((PSC+1)*(ARR+1))
  return tim_ker_clk / ((psc + 1) * (arr + 1));
}

static const pwm_driver_ops_t stm32_pwm_ops = {
    .start = _stm32_pwm_start,
    .stop = _stm32_pwm_stop,
    .set_duty = _stm32_pwm_set_duty,
    .set_freq = _stm32_pwm_set_freq,
    .get_freq = _stm32_pwm_get_freq,
    .get_duty_max = _stm32_pwm_get_duty_max};

stm32_pwm_driver_t *stm32_pwm_driver_create(TIM_HandleTypeDef *htim,
                                            uint32_t channel)
{
  stm32_pwm_driver_t *self =
      (stm32_pwm_driver_t *)malloc(sizeof(stm32_pwm_driver_t));
  if (self)
  {
    self->base.ops = &stm32_pwm_ops;
    self->htim = htim;
    self->channel = channel;
  }
  return self;
}
