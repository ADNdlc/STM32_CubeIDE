/*
 * pwm_led.c
 *
 *  Created on: Nov 26, 2025
 *      Author: 12114
 */

#include "pwm_led.h"
#include "sys.h"
#include <stdint.h>
#include <stdlib.h>
#include "MemPool.h"
#ifdef USE_MEMPOOL
#define PWMLED_MEMSOURCE SYS_MEM_INTERNAL
#else
#define PWMLED_MEMSOURCE 0 // 标记不使用内存池
#endif

/* ==========================================
 * 构造与初始化
 * ========================================== */

void pwm_led_init(pwm_led_t *self, pwm_driver_t *pwm_driver, uint8_t active_level) {
  if (!self || !pwm_driver)
    return;
  self->pwm_driver = pwm_driver;
  self->active_level = active_level;
  self->current_duty = 0;

  // 默认启动 PWM (频率可能已由驱动层或工厂层预设)
  PWM_START(self->pwm_driver);
  // 初始状态为关闭 (亮度0)
  pwm_led_set_brightness(self, 0);
}

pwm_led_t *pwm_led_create(uint32_t freq, pwm_driver_t *pwm_driver, uint8_t active_level) {
  if (!pwm_driver)
    return NULL;

  pwm_led_t *self;
#ifdef USE_MEMPOOL
  self = (pwm_led_t *)sys_malloc(PWMLED_MEMSOURCE, sizeof(pwm_led_t));
#else
  self = (pwm_led_t *)malloc(sizeof(pwm_led_t));
#endif

  if (self) {
    // 先设置频率
    PWM_SET_FREQ(pwm_driver, freq);
    // 初始化组件
    pwm_led_init(self, pwm_driver, active_level);
  }
  return self;
}

void pwm_led_destroy(pwm_led_t *self) {
  if (self) {
    // 停止PWM
    if (self->pwm_driver) {
      PWM_STOP(self->pwm_driver);
    }
#ifdef USE_MEMPOOL
    sys_free(PWMLED_MEMSOURCE, self);
#else
    free(self);
#endif
  }
}

void pwm_led_set_state(pwm_led_t *self, uint8_t state) {
  if (!self)
    return;
  // 这里的 state：1=ON (100% 亮度), 0=OFF (0% 亮度)
  pwm_led_set_brightness(self, state ? 100 : 0);
}

uint8_t pwm_led_get_state(pwm_led_t *self) {
  if (!self)
    return 0;
  return self->current_duty > 0 ? 1 : 0;
}

void pwm_led_set_brightness(pwm_led_t *self, uint32_t brightness) {
  if (!self || !self->pwm_driver)
    return;

  if (brightness > 100)
    brightness = 100;

  self->current_duty = brightness;

  // 获取驱动层的最大占空比 (ARR值)
  uint32_t max_duty = PWM_GET_DUTY_MAX(self->pwm_driver);

  // 计算实际写入寄存器的对比值
  // 亮度 0-100 映射到 0-max_duty
  // 如果 active_level 为 0 (低电平有效)，则需要反转占空比
  uint32_t target_duty;
  if (self->active_level) { // 高电平有效
    target_duty = (brightness * max_duty) / 100;
  } else { // 低电平有效
    target_duty = max_duty - ((brightness * max_duty) / 100);
  }

  PWM_SET_DUTY(self->pwm_driver, target_duty);
}

uint32_t pwm_led_get_brightness(pwm_led_t *self) {
  if (!self)
    return 0;
  return self->current_duty;
}
