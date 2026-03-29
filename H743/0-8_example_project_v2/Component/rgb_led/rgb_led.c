#include "rgb_led.h"
#include "Sys.h"
#include "MemPool.h"
#include <stdlib.h>
#include <stdint.h>

#ifdef USE_MEMPOOL
#define RGBLED_MEMSOURCE SYS_MEM_INTERNAL
#else
#define RGBLED_MEMSOURCE 0
#endif

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

void rgb_led_init(rgb_led_t *self, pwm_led_t *red, pwm_led_t *green, pwm_led_t *blue) {
  if (!self)
    return;
  self->red = red;
  self->green = green;
  self->blue = blue;
  self->current_color = 0;
  rgb_led_set_color(self, 0); // 初始关闭
}

rgb_led_t *rgb_led_create(pwm_led_t *red, pwm_led_t *green, pwm_led_t *blue) {
  rgb_led_t *self;
#ifdef USE_MEMPOOL
  self = (rgb_led_t *)sys_malloc(RGBLED_MEMSOURCE, sizeof(rgb_led_t));
#else
  self = (rgb_led_t *)malloc(sizeof(rgb_led_t));
#endif

  if (self) {
    rgb_led_init(self, red, green, blue);
  }
  return self;
}

void rgb_led_destroy(rgb_led_t *self) {
  if (self) {
#ifdef USE_MEMPOOL
    sys_free(RGBLED_MEMSOURCE, self);
#else
    free(self);
#endif
  }
}

void rgb_led_set_color(rgb_led_t *self, uint32_t color) {
  if (!self)
    return;

  self->current_color = color;

  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;

  // 将 0-255 映射到 pwm_led 的 0-100 亮度
  if (self->red)
    pwm_led_set_brightness(self->red, (r * 100) / 255);
  if (self->green)
    pwm_led_set_brightness(self->green, (g * 100) / 255);
  if (self->blue)
    pwm_led_set_brightness(self->blue, (b * 100) / 255);
}

void rgb_led_set_hsv(rgb_led_t *self, uint16_t h, uint8_t s, uint8_t v) {
  if (!self)
    return;
  uint32_t color = _hsv_to_rgb(h, s, v);
  rgb_led_set_color(self, color);
}

