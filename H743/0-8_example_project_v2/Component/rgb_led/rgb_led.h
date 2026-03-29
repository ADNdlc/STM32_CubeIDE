#ifndef RGB_LED_RGB_LED_H_
#define RGB_LED_RGB_LED_H_

#include "pwm_led.h"

struct rgb_led_t {
  // rgb_led 类成员变量(聚合)
  pwm_led_t *red;   // 红色 PWM LED
  pwm_led_t *green; // 绿色 PWM LED
  pwm_led_t *blue;  // 蓝色 PWM LED

  uint32_t current_color; // 当前颜色 (0x00RRGGBB)
};

#endif /* RGB_LED_RGB_LED_H_ */
