#include "rgb_led.h"

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

