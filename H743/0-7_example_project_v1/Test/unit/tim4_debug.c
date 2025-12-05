/*
 * tim4_debug.c
 * TIM4 PWM调试辅助文件
 * 用于诊断RGB LED不亮的问题
 */

#include "stm32h7xx_hal.h"
#include "tim.h"


// 诊断函数：检查TIM4的寄存器状态
void tim4_diagnose(void) {
  // 1. 检查时钟是否使能
  uint32_t tim4_clk_enabled = __HAL_RCC_TIM4_IS_CLK_ENABLED();

  // 2. 检查TIM4寄存器值
  uint32_t cr1 = TIM4->CR1;     // 控制寄存器1
  uint32_t psc = TIM4->PSC;     // 预分频器
  uint32_t arr = TIM4->ARR;     // 自动重载值
  uint32_t ccr1 = TIM4->CCR1;   // 通道1比较值
  uint32_t ccr2 = TIM4->CCR2;   // 通道2比较值
  uint32_t ccr3 = TIM4->CCR3;   // 通道3比较值
  uint32_t ccer = TIM4->CCER;   // 捕获/比较使能寄存器
  uint32_t ccmr1 = TIM4->CCMR1; // 捕获/比较模式寄存器1
  uint32_t ccmr2 = TIM4->CCMR2; // 捕获/比较模式寄存器2

  // 3. 检查GPIO配置
  uint32_t gpiod_moder = GPIOD->MODER; // PD12, PD13
  uint32_t gpiod_afrh = GPIOD->AFR[1]; // PD12, PD13的复用功能
  uint32_t gpiob_moder = GPIOB->MODER; // PB8
  uint32_t gpiob_afrh = GPIOB->AFR[1]; // PB8的复用功能

  // 在此设置断点，查看各寄存器的值
  // 正常情况下应该看到：
  // - tim4_clk_enabled = 1
  // - cr1 & 0x01 = 1 (CEN位，计数器使能)
  // - ccer & 0x0111 = 0x0111 (CC1E, CC2E, CC3E位，通道输出使能)
  // - ccmr1 = 0x6868 (PWM模式1)
  // - ccmr2 = 0x0068 (PWM模式1)

  (void)tim4_clk_enabled;
  (void)cr1;
  (void)psc;
  (void)arr;
  (void)ccr1;
  (void)ccr2;
  (void)ccr3;
  (void)ccer;
  (void)ccmr1;
  (void)ccmr2;
  (void)gpiod_moder;
  (void)gpiod_afrh;
  (void)gpiob_moder;
  (void)gpiob_afrh;
}

// 手动启动TIM4的所有PWM通道（用于测试）
void tim4_force_start_all_channels(void) {
  // 启动所有三个通道
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);

  // 设置一个中等占空比用于测试（50%）
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 32768);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 32768);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 32768);
}
