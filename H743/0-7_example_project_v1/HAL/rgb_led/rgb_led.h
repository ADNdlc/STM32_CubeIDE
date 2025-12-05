/*
 * rgb_led.h
 *
 *  Created on: Nov 30, 2025
 *      Author: 12114
 */

#ifndef HAL_RGB_LED_RGB_LED_H_
#define HAL_RGB_LED_RGB_LED_H_

#include "led_hal.h"
#include "pwm_led/pwm_led.h"
#include <stdint.h>

// 前向声明
typedef struct rgb_led_t rgb_led_t;

// 定义虚函数表类型
typedef struct {
  led_hal_vtable_t base_vtable;
} rgb_led_vtable_t;

// RGB LED 结构体（继承自 led_hal，聚合了三个 pwm_led）
struct rgb_led_t {
  led_hal_t base; // 继承自 led_hal_t 基类

  // rgb_led 类成员变量(聚合)
  pwm_led_t *red;   // 红色 PWM LED
  pwm_led_t *green; // 绿色 PWM LED
  pwm_led_t *blue;  // 蓝色 PWM LED

  uint32_t current_color; // 当前颜色 (0x00RRGGBB)
};

// 公共 API
void rgb_led_init(rgb_led_t *self, pwm_led_t *red, pwm_led_t *green,
                  pwm_led_t *blue);
rgb_led_t *rgb_led_create(pwm_led_t *red, pwm_led_t *green, pwm_led_t *blue);
void rgb_led_delete(rgb_led_t *self);

/**
 * @brief 设置RGB LED颜色
 * @param self rgb_led_t 指针
 * @param color 颜色值 (0x00RRGGBB)
 */
static inline void rgb_led_set_color(rgb_led_t *self, uint32_t color) {
  led_hal_set_data((led_hal_t *)self, color);
}

/**
 * @brief 使用HSV颜色模型设置RGB LED颜色
 * @param self rgb_led_t 指针
 * @param h 色相 (0-359) - 色环角度，0=红，120=绿，240=蓝
 * @param s 饱和度 (0-100) - 颜色纯度，0=灰色，100=纯色
 * @param v 明度 (0-100) - 亮度，0=黑色，100=最亮
 */
void rgb_led_set_hsv(rgb_led_t *self, uint16_t h, uint8_t s, uint8_t v);

#endif /* HAL_RGB_LED_RGB_LED_H_ */
