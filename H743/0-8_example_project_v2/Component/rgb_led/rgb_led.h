#ifndef RGB_LED_RGB_LED_H_
#define RGB_LED_RGB_LED_H_

#include "pwm_led/pwm_led.h"

typedef struct rgb_led_t rgb_led_t;

struct rgb_led_t {
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
void rgb_led_destroy(rgb_led_t *self);

/**
 * @brief 设置RGB颜色
 * @param self rgb_led_t 指针
 * @param color 颜色值 (0x00RRGGBB)
 */
void rgb_led_set_color(rgb_led_t *self, uint32_t color);

/**
 * @brief 使用HSV设置颜色
 * @param self rgb_led_t 指针
 * @param h 色相 (0-359)
 * @param s 饱和度 (0-100)
 * @param v 明度 (0-100)
 */
void rgb_led_set_hsv(rgb_led_t *self, uint16_t h, uint8_t s, uint8_t v);

#endif /* RGB_LED_RGB_LED_H_ */
