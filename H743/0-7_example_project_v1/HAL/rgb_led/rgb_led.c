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
static void _rgb_led_set_hsv(rgb_led_t *self, uint16_t h, uint8_t s, uint8_t v);

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

/**
 * @brief HSV转RGB颜色空间转换
 * @param h 色相 (0-359)
 * @param s 饱和度 (0-100)
 * @param v 明度 (0-100)
 * @return RGB888格式颜色值 (0x00RRGGBB)
 */
static uint32_t _hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v) {
  uint8_t r, g, b;

  // 限制输入范围
  if (h >= 360)
    h = 359;
  if (s > 100)
    s = 100;
  if (v > 100)
    v = 100;

  // 如果饱和度为0，则为灰度
  if (s == 0) {
    r = g = b = (v * 255) / 100;
    return (r << 16) | (g << 8) | b;
  }

  // HSV to RGB 转换算法
  uint16_t region = h / 60;
  uint16_t remainder = (h - (region * 60)) * 6;

  uint8_t p = (v * (100 - s)) / 100;
  uint8_t q = (v * (100 - ((s * remainder) / 360))) / 100;
  uint8_t t = (v * (100 - ((s * (360 - remainder)) / 360))) / 100;

  // 转换为0-255范围
  uint8_t v_scaled = (v * 255) / 100;
  p = (p * 255) / 100;
  q = (q * 255) / 100;
  t = (t * 255) / 100;

  switch (region) {
  case 0:
    r = v_scaled;
    g = t;
    b = p;
    break;
  case 1:
    r = q;
    g = v_scaled;
    b = p;
    break;
  case 2:
    r = p;
    g = v_scaled;
    b = t;
    break;
  case 3:
    r = p;
    g = q;
    b = v_scaled;
    break;
  case 4:
    r = t;
    g = p;
    b = v_scaled;
    break;
  default: // case 5:
    r = v_scaled;
    g = p;
    b = q;
    break;
  }

  return (r << 16) | (g << 8) | b;
}

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

  // 如果当前颜色为0，设置占空比为0（关闭状态）
  // 否则恢复之前设置的颜色
  if (self->current_color == 0) {
    // 设置占空比为0，但仍然启动PWM通道
    _rgb_led_set_rgb(base, 0x000000);
  } else {
    // 恢复之前的颜色
    _rgb_led_set_rgb(base, self->current_color);
  }

  // 启动所有PWM通道（关键修复）
  if (self->red) {
    self->red->base.vtable->on((led_hal_t *)self->red);
  }
  if (self->green) {
    self->green->base.vtable->on((led_hal_t *)self->green);
  }
  if (self->blue) {
    self->blue->base.vtable->on((led_hal_t *)self->blue);
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

/**
 * @brief 使用HSV颜色模型设置RGB LED颜色
 * @param self rgb_led_t 指针
 * @param h 色相 (0-359)
 * @param s 饱和度 (0-100)
 * @param v 明度 (0-100)
 */
static void _rgb_led_set_hsv(rgb_led_t *self, uint16_t h, uint8_t s,
                             uint8_t v) {
  // 转换HSV到RGB
  uint32_t rgb = _hsv_to_rgb(h, s, v);
  // 调用RGB设置函数
  _rgb_led_set_rgb((led_hal_t *)self, rgb);
}

/* ==========================================
 * 公共接口函数
 * ========================================== */

/**
 * @brief 使用HSV颜色模型设置RGB LED颜色（公共接口）
 * @param self rgb_led_t 指针
 * @param h 色相 (0-359) - 色环角度，0=红，120=绿，240=蓝
 * @param s 饱和度 (0-100) - 颜色纯度，0=灰色，100=纯色
 * @param v 明度 (0-100) - 亮度，0=黑色，100=最亮
 */
void rgb_led_set_hsv(rgb_led_t *self, uint16_t h, uint8_t s, uint8_t v) {
  if (self) {
    _rgb_led_set_hsv(self, h, s, v);
  }
}
