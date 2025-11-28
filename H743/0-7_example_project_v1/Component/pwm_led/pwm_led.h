/*
 * pwm_led.h
 *
 *  Created on: Nov 26, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_PWM_LED_PWM_LED_H_
#define BSP_DEVICE_DRIVER_PWM_LED_PWM_LED_H_

#include <stdint.h>
#include "pwm_driver.h"

// 前向声明
typedef struct pwm_led_t pwm_led_t;

// 定义虚函数表类型
typedef struct {
    void (*on)(pwm_led_t *self);
    void (*off)(pwm_led_t *self);
    void (*set_brightness)(pwm_led_t *self, uint16_t brightness);
    uint16_t (*get_brightness)(pwm_led_t *self);
} pwm_led_vtable_t;

// PWM LED 结构体（与 led 同级的基类）
struct pwm_led_t {
    const pwm_led_vtable_t *vtable; // 虚表指针必须在第一位
    
    // pwm_led 类成员变量
    pwm_driver_t *pwm_driver;
    uint32_t current_duty;
};

// 公共 API
void pwm_led_init(pwm_led_t *self, uint32_t freq, pwm_driver_t *pwm_driver);
pwm_led_t* pwm_led_create(uint32_t freq, pwm_driver_t *pwm_driver);
void pwm_led_delete(pwm_led_t *self);

// 多态调用接口
void pwm_led_on(pwm_led_t *self);
void pwm_led_off(pwm_led_t *self);
void pwm_led_set_brightness(pwm_led_t *self, uint32_t duty);
uint8_t pwm_led_get_state(pwm_led_t *self);

#endif /* BSP_DEVICE_DRIVER_PWM_LED_PWM_LED_H_ */