/*
 * pwm_led.h
 *
 *  Created on: Nov 26, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_PWM_LED_PWM_LED_H_
#define BSP_DEVICE_DRIVER_PWM_LED_PWM_LED_H_

#include "interface_inc.h"
#include <stdint.h>

typedef struct pwm_led_t pwm_led_t;

// pwm_led 结构体
struct pwm_led_t
{
  pwm_driver_t *pwm_driver; // 驱动
  uint32_t current_duty;    // 当前亮度
  uint8_t active_level;     // 有效电平
};

// 公共 API
void pwm_led_init(pwm_led_t *self, pwm_driver_t *pwm_driver,
                  uint8_t active_level);
pwm_led_t *pwm_led_create(uint32_t freq, pwm_driver_t *pwm_driver,
                          uint8_t active_level);
void pwm_led_destroy(pwm_led_t *self);

/**
 * @brief 开关
 * @param self pwm_led_t 实例
 * @param state 1=ON, 0=OFF
 */
void pwm_led_set_state(pwm_led_t *self, uint8_t state);
/**
 * @brief 获取开关状态
 * @param self pwm_led_t 实例
 * @return state 1=ON, 0=OFF
 */
uint8_t pwm_led_get_state(pwm_led_t *self);

/**
 * @brief 设置PWM LED亮度
 * @param self pwm_led_t 指针
 * @param brightness 亮度值 (0-100)
 */
void pwm_led_set_brightness(pwm_led_t *self, uint32_t brightness);
/**
 * @brief 获取PWM LED亮度
 * @param self pwm_led_t 指针
 * @return 亮度值 (0-100)
 */
uint32_t pwm_led_get_brightness(pwm_led_t *self);

#endif /* BSP_DEVICE_DRIVER_PWM_LED_PWM_LED_H_ */
