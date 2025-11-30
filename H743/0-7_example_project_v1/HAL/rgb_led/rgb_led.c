/*
 * rgb_led.c
 *
 *  Created on: Nov 30, 2025
 *      Author: 12114
 */

#include "rgb_led.h"
#include <stdlib.h>

// 私有函数声明
static void _rgb_led_on(led_hal_t *base);
static void _rgb_led_off(led_hal_t *base);
static void _rgb_led_set_rgb(led_hal_t *base, uint32_t data);
static uint32_t _rgb_led_get_rgb(led_hal_t *base);

// 虚函数表实例
static const rgb_led_vtable_t _rgb_led_vtable = {
    .base_vtable = {
        .on = _rgb_led_on,
        .off = _rgb_led_off,
        .set_data = _rgb_led_set_rgb,
        .get_data = _rgb_led_get_rgb,
    }};

/* ==========================================
 * 构造与初始化
 * ========================================== */
void rgb_led_init(rgb_led_t *self, pwm_led_t *red, pwm_led_t *green,
                  pwm_led_t *blue) {
  self->base.vtable = (led_hal_vtable_t *)&_rgb_led_vtable;
  self->red = red;
  self->green = green;
  self->blue = blue;
  self->current_color = 0; // 默认为关闭
}

rgb_led_t *rgb_led_create(pwm_led_t *red, pwm_led_t *green, pwm_led_t *blue) {
  rgb_led_t *self = (rgb_led_t *)malloc(sizeof(rgb_led_t));
  if (self) {
    rgb_led_init(self, red, green, blue);
  }
  return self;
}

void rgb_led_delete(rgb_led_t *self) {
  if (self) {
    // rgb_led 不负责销毁传入的 pwm_led
    free(self);
  }
}

/* ==========================================
 * 接口实现
 * ========================================== */

static void _rgb_led_set_rgb(led_hal_t *base, uint32_t data) {
  rgb_led_t *self = (rgb_led_t *)base;
  self->current_color = data;

  // 解析 RGB888 (0x00RRGGBB)
  uint8_t r = (data >> 16) & 0xFF;
  uint8_t g = (data >> 8) & 0xFF;
  uint8_t b = data & 0xFF;

  // 映射 0-255 到 0-1000 (pwm_led 的亮度范围)
  // 简单的线性映射: val * 1000 / 255
  // 为了避免浮点运算，使用整数运算
  uint32_t r_val = ((uint32_t)r * 1000) / 255;
  uint32_t g_val = ((uint32_t)g * 1000) / 255;
  uint32_t b_val = ((uint32_t)b * 1000) / 255;

  // 设置各个通道的亮度
  if (self->red) {
    self->red->base.vtable->set_data((led_hal_t *)self->red, r_val);
  }
  if (self->green) {
    self->green->base.vtable->set_data((led_hal_t *)self->green, g_val);
  }
  if (self->blue) {
    self->blue->base.vtable->set_data((led_hal_t *)self->blue, b_val);
  }
}

static void _rgb_led_on(led_hal_t *base) {
  rgb_led_t *self = (rgb_led_t *)base;
  // 恢复之前的颜色，如果之前是0，则默认全白
  if (self->current_color == 0) {
    _rgb_led_set_rgb(base, 0xFFFFFF);
  } else {
    _rgb_led_set_rgb(base, self->current_color);
  }
}

static void _rgb_led_off(led_hal_t *base) {
  rgb_led_t *self = (rgb_led_t *)base;
  // 关闭所有通道，但不清除 current_color，以便 on() 恢复
  if (self->red) {
    self->red->base.vtable->off((led_hal_t *)self->red);
  }
  if (self->green) {
    self->green->base.vtable->off((led_hal_t *)self->green);
  }
  if (self->blue) {
    self->blue->base.vtable->off((led_hal_t *)self->blue);
  }
}

static uint32_t _rgb_led_get_rgb(led_hal_t *base) {
  rgb_led_t *self = (rgb_led_t *)base;
  return self->current_color;
}
