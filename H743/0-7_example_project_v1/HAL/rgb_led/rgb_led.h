/*
 * rgb_led.h
 *
 *  Created on: Nov 29, 2025
 *      Author: 12114
 */

#ifndef COMPONENT_RGB_LED_RGB_LED_H_
#define COMPONENT_RGB_LED_RGB_LED_H_

#include <stdint.h>
#include "pwm_led.h"

// 前向声明
typedef struct rgb_led_t rgb_led_t;

// 定义虚函数表类型
typedef struct{
    pwm_led_vtable_t base_vtable;
    void (*set_rgb)(rgb_led_t *self, uint16_t r, uint16_t g, uint16_t b);
    void (*get_rgb)(rgb_led_t *self, uint16_t* r, uint16_t* g, uint16_t* b);
} rgb_led_vtable_t;

// RGB LED 结构体（与 pwm_led 同级的基类）
struct rgb_led_t {
    const rgb_led_vtable_t *vtable; //虚表指针

    // rgb_led 类成员变量
    pwm_led_t *pwm_led_r; // 红色 PWM LED
    pwm_led_t *pwm_led_g; // 绿色 PWM LED
    pwm_led_t *pwm_led_b; // 蓝色 PWM LED
}

#endif /* COMPONENT_RGB_LED_RGB_LED_H_ */
