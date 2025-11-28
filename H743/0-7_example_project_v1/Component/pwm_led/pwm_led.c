/*
 * pwm_led.c
 *
 *  Created on: Nov 26, 2025
 *      Author: 12114
 */

#include "pwm_led.h"
#include <stdlib.h>

// 重写基类虚函数
static void _pwm_led_on(led_t *base) {
  pwm_led_t *self = (pwm_led_t *)base;
  // 开启 PWM，恢复之前的亮度
  PWM_START(self->pwm_driver);
  PWM_SET_DUTY(self->pwm_driver, self->current_duty);
}

static void _pwm_led_off(led_t *base) {
  pwm_led_t *self = (pwm_led_t *)base;
  // 停止 PWM
  PWM_STOP(self->pwm_driver);
}

static void _pwm_led_toggle(led_t *base) {
  // PWM LED toggle 逻辑比较模糊，这里简单定义为：如果是开的就关，关的就开
  // 或者可以在 0 和 current_duty 之间切换
  // 这里暂时不做复杂实现，留空或简单调用 on/off
  // 更好的做法可能需要记录 state
}

static uint8_t _pwm_led_get_state(led_t *base) {
  // 需要记录状态，这里简化处理
  return 0;
}

// 子类特有虚函数实现
static void _pwm_led_set_brightness(pwm_led_t *self, uint32_t duty) {
  self->current_duty = duty;
  PWM_SET_DUTY(self->pwm_driver, duty);
}

// 虚函数表实例
static const pwm_led_vtable_t pwm_led_vtable = {
    .base_vtable =
        {
            .on = _pwm_led_on,
            .off = _pwm_led_off,
            .toggle = _pwm_led_toggle,
            .get_state = _pwm_led_get_state,
        },
    .set_brightness = _pwm_led_set_brightness,
};

pwm_led_t *pwm_led_create(void *port, uint16_t pin,
                          const led_gpio_ops_t *gpio_ops,
                          pwm_driver_t *pwm_driver) {
  pwm_led_t *self = (pwm_led_t *)malloc(sizeof(pwm_led_t));
  if (self) {
    // 初始化基类
    led_init(&self->base, port, pin, gpio_ops,
             1); // 默认 active_level 1，可调整

    // 覆盖虚表指针
    self->base.vtable = (const led_vtable_t *)&pwm_led_vtable;

    // 初始化子类成员
    self->pwm_driver = pwm_driver;
    self->current_duty = 0;
  }
  return self;
}

void pwm_led_set_brightness(pwm_led_t *self, uint32_t duty) {
  if (self && self->base.vtable) {
    // 安全检查，确保是 pwm_led_t 类型 (在 C 中比较难完全保证，依赖调用者)
    // 调用虚函数
    const pwm_led_vtable_t *vtable =
        (const pwm_led_vtable_t *)self->base.vtable;
    if (vtable->set_brightness) {
      vtable->set_brightness(self, duty);
    }
  }
}
