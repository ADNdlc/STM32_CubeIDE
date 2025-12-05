/*
 * pwm_led.c
 *
 *  Created on: Nov 26, 2025
 *      Author: 12114
 */

#include "pwm_led.h"
#include <stdint.h>
#include <stdlib.h>

// 私有函数声明
static void _pwm_led_on(led_hal_t *base);
static void _pwm_led_off(led_hal_t *base);
static void _pwm_led_set_brightness(led_hal_t *base, uint32_t brightness);
static uint32_t _pwm_led_get_brightness(led_hal_t *base);

// pwm_led类虚表实例
static const pwm_led_vtable_t _pwm_led_vtable = {
    .base_vtable = {
        .on = _pwm_led_on,
        .off = _pwm_led_off,
        .set_data = _pwm_led_set_brightness,
        .get_data = _pwm_led_get_brightness,
    }};

/* ==========================================
 * 构造与初始化
 * ========================================== */

void pwm_led_init(pwm_led_t *self, uint32_t freq, uint8_t active_level,
                  pwm_driver_t *pwm_driver) {
  self->base.vtable = (led_hal_vtable_t *)&_pwm_led_vtable;
  self->pwm_driver = pwm_driver;
  self->current_duty = 0;
  self->active_level = active_level;
  // 设置频率
  if (pwm_driver && pwm_driver->ops && pwm_driver->ops->set_freq) {
    PWM_SET_FREQ(pwm_driver, freq);
  }
}

pwm_led_t *pwm_led_create(uint32_t freq, uint8_t active_level,
                          pwm_driver_t *pwm_driver) {
  pwm_led_t *self = (pwm_led_t *)malloc(sizeof(pwm_led_t));
  if (self) {
    pwm_led_init(self, freq, active_level, pwm_driver);
  }
  return self;
}

void pwm_led_delete(pwm_led_t *self) {
  if (self) {
    // 停止PWM
    if (self->pwm_driver && self->pwm_driver->ops &&
        self->pwm_driver->ops->stop) {
      PWM_STOP(self->pwm_driver);
    }
    free(self);
  }
}

/* ==========================================
 * 接口实现
 * ========================================== */

static void _pwm_led_on(led_hal_t *base) {
  pwm_led_t *self = (pwm_led_t *)base;
  PWM_START(self->pwm_driver);
}

static void _pwm_led_off(led_hal_t *base) {
  pwm_led_t *self = (pwm_led_t *)base;
  PWM_STOP(self->pwm_driver);
}

/* 设置亮度
 * brightness 亮度值(0~1000)
 */
static void _pwm_led_set_brightness(led_hal_t *base, uint32_t brightness) {
  pwm_led_t *self = (pwm_led_t *)base;
  // 根据分辨率映射到占空比
  uint32_t duty_max = PWM_GET_DUTY_MAX(self->pwm_driver);
  // 限制亮度值在有效范围内
  if (brightness > 1000) {
    brightness = 1000;
  }
  // 计算实际占空比值
  uint32_t duty = ((uint32_t)brightness * duty_max) / 1000;

  // 如果是低电平有效，反转占空比
  if (self->active_level == 0) {
    duty = duty_max - duty;
  }

  // 设置占空比
  PWM_SET_DUTY(self->pwm_driver, duty);
  // 更新当前亮度值
  self->current_duty = duty;
}

/* 获取亮度
 * 返回值: 亮度值(0~1000)
 */
static uint32_t _pwm_led_get_brightness(led_hal_t *base) {
  pwm_led_t *self = (pwm_led_t *)base;
  uint32_t duty_max = PWM_GET_DUTY_MAX(self->pwm_driver);
  if (duty_max == 0) {
    return 0;
  }

  uint32_t duty = self->current_duty;
  if (self->active_level == 0) {
    duty = duty_max - duty;
  }

  return (duty * 1000) / duty_max;
}
