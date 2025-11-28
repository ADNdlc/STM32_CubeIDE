/*
 * pwm_led.h
 *
 *  Created on: Nov 26, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_PWM_LED_PWM_LED_H_
#define BSP_DEVICE_DRIVER_PWM_LED_PWM_LED_H_

#include "led/led.h"
#include "pwm_driver.h"

// 子类结构体
typedef struct pwm_led_t {
  led_t base;
  // 依赖注入：持有抽象的 PWM 驱动指针
  pwm_driver_t *pwm_driver;
  uint32_t current_duty;
} pwm_led_t;

// 子类虚函数表 (扩展)
typedef struct {
  led_vtable_t base_vtable; //父类方法
  void (*set_brightness)(struct pwm_led_t *self, uint32_t duty); // 新增方法
} pwm_led_vtable_t;

// 子类构造函数
pwm_led_t *pwm_led_create(uint32_t freq, pwm_driver_t *pwm_driver);

// 子类特有方法
void pwm_led_set_brightness(pwm_led_t *self, uint32_t duty);

#endif /* BSP_DEVICE_DRIVER_PWM_LED_PWM_LED_H_ */
