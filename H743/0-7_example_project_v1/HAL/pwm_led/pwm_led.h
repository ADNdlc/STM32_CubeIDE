/*
 * pwm_led.h
 *
 *  Created on: Nov 26, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_PWM_LED_PWM_LED_H_
#define BSP_DEVICE_DRIVER_PWM_LED_PWM_LED_H_

#include "led_hal.h"
#include "pwm_driver.h"
#include <stdint.h>

//从哪里分配
#define PWMLED_MEMSOURCE  SYS_MEM_INTERNAL

// 前向声明
typedef struct pwm_led_t pwm_led_t;

// 定义虚函数表类型
typedef struct {
  led_hal_vtable_t base_vtable;
} pwm_led_vtable_t;

// pwm_led 类结构体
struct pwm_led_t {
  led_hal_t base; // 继承接口行为

  // pwm_led 类成员变量
  pwm_driver_t *pwm_driver; // 依赖的驱动
  uint32_t current_duty;    // 当前亮度(data)
  uint8_t active_level;     // 有效电平
};

// 公共 API
void pwm_led_init(pwm_led_t *self, uint32_t freq, uint8_t active_level,
                  pwm_driver_t *pwm_driver);
pwm_led_t *pwm_led_create(uint32_t freq, uint8_t active_level,
                          pwm_driver_t *pwm_driver);
void pwm_led_delete(pwm_led_t *self);

/**
 * @brief 设置PWM LED亮度
 * @param self pwm_led_t 指针
 * @param brightness 亮度值 (0-1000)
 */
static inline void pwm_led_set_brightness(pwm_led_t *self,
                                          uint32_t brightness) {
  led_hal_set_data((led_hal_t *)self, brightness);
}

#endif /* BSP_DEVICE_DRIVER_PWM_LED_PWM_LED_H_ */
