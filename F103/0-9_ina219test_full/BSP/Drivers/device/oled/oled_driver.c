/*
 * oled_driver.c
 *
 *  Created on: Mar 2, 2026
 *      Author: 12114
 */

#include "oled_driver.h"
#include <string.h>
#include <stdlib.h>

/**
 * @brief 初始化 OLED
 */
void OLED_Init(oled_device_t *dev) {
  if (dev && dev->ops && dev->ops->init) {
    dev->ops->init(dev);
    OLED_Clear(dev, OLED_COLOR_NORMAL);
    OLED_ReFresh(dev);
  }
}

/**
 * @brief 开启显示
 */
void OLED_DisPlay_On(oled_device_t *dev) {
  if (dev && dev->ops && dev->ops->display_on) {
    dev->ops->display_on(dev);
  }
}

/**
 * @brief 关闭显示
 */
void OLED_DisPlay_Off(oled_device_t *dev) {
  if (dev && dev->ops && dev->ops->display_off) {
    dev->ops->display_off(dev);
  }
}

/**
 * @brief 将缓冲区内容刷新到屏幕
 */
void OLED_ReFresh(oled_device_t *dev) {
  if (!dev || !dev->buffer || !dev->ops->set_cursor)
    return;

  uint8_t pages = dev->height / 8;
  for (uint8_t i = 0; i < pages; i++) {
    dev->ops->set_cursor(dev, i, 0);
    dev->bus->write(dev->bus, OLED_DATA, &dev->buffer[i * dev->width], dev->width);
  }
}

/**
 * @brief 清空显存
 * @param mode 0: 清除(黑底), 1: 填充(白底)
 */
void OLED_Clear(oled_device_t *dev, oled_color_t mode) {
  if (!dev || !dev->buffer)
    return;
  memset(dev->buffer, (mode == OLED_COLOR_NORMAL) ? 0x00 : 0xFF,
         (dev->height / 8) * dev->width);
}

#if OLED_CFG_USE_GRAPHICS

/**
 * @brief 画点
 */
void OLED_DrawPoint(oled_device_t *dev, uint8_t x, uint8_t y, oled_color_t mode) {
  if (!dev || !dev->buffer || x >= dev->width || y >= dev->height)
    return;

  if (mode == OLED_COLOR_NORMAL) {
    dev->buffer[(y / 8) * dev->width + x] |= (1 << (y % 8));
  } else {
    dev->buffer[(y / 8) * dev->width + x] &= ~(1 << (y % 8));
  }
}

/**
 * @brief 画线 (Bresenham算法)
 */
void OLED_DrawLine(oled_device_t *dev, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, oled_color_t color) {
  int16_t dx = x2 - x1;
  int16_t dy = y2 - y1;
  int16_t ux = ((dx > 0) << 1) - 1;
  int16_t uy = ((dy > 0) << 1) - 1;
  int16_t x = x1, y = y1, eps = 0;
  dx = abs(dx);
  dy = abs(dy);
  if (dx > dy) {
    for (x = x1; x != x2; x += ux) {
      OLED_DrawPoint(dev, x, y, color);
      eps += dy;
      if ((eps << 1) >= dx) {
        y += uy;
        eps -= dx;
      }
    }
  } else {
    for (y = y1; y != y2; y += uy) {
      OLED_DrawPoint(dev, x, y, color);
      eps += dx;
      if ((eps << 1) >= dy) {
        x += ux;
        eps -= dy;
      }
    }
  }
  OLED_DrawPoint(dev, x2, y2, color);
}

void OLED_DrawRectangle(oled_device_t *dev, uint8_t x, uint8_t y, uint8_t w, uint8_t h, oled_color_t color) {
  OLED_DrawLine(dev, x, y, x + w - 1, y, color);
  OLED_DrawLine(dev, x, y + h - 1, x + w - 1, y + h - 1, color);
  OLED_DrawLine(dev, x, y, x, y + h - 1, color);
  OLED_DrawLine(dev, x + w - 1, y, x + w - 1, y + h - 1, color);
}

void OLED_DrawFilledRectangle(oled_device_t *dev, uint8_t x, uint8_t y, uint8_t w, uint8_t h, oled_color_t color) {
  for (uint8_t i = 0; i < h; i++) {
    OLED_DrawLine(dev, x, y + i, x + w - 1, y + i, color);
  }
}

void OLED_DrawTriangle(oled_device_t *dev, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, oled_color_t color) {
  OLED_DrawLine(dev, x1, y1, x2, y2, color);
  OLED_DrawLine(dev, x2, y2, x3, y3, color);
  OLED_DrawLine(dev, x3, y3, x1, y1, color);
}

void OLED_DrawFilledTriangle(oled_device_t *dev, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, oled_color_t color) {
  // 简化实现：通过扫描线填充，或者直接调用库里逻辑
  // 这里采用更健壮的点阵扫描法以适应任意三角形
  uint8_t min_x = x1, max_x = x1, min_y = y1, max_y = y1;
  if(x2 < min_x) min_x = x2; if(x2 > max_x) max_x = x2;
  if(x3 < min_x) min_x = x3; if(x3 > max_x) max_x = x3;
  if(y2 < min_y) min_y = y2; if(y2 > max_y) max_y = y2;
  if(y3 < min_y) min_y = y3; if(y3 > max_y) max_y = y3;

  for (uint8_t i = min_x; i <= max_x; i++) {
    for (uint8_t j = min_y; j <= max_y; j++) {
      // 通过重心坐标法判断点是否在三角形内
      int16_t x = i, y = j;
      int32_t a = (int32_t)(x1-x)*(y2-y) - (int32_t)(x2-x)*(y1-y);
      int32_t b = (int32_t)(x2-x)*(y3-y) - (int32_t)(x3-x)*(y2-y);
      int32_t c = (int32_t)(x3-x)*(y1-y) - (int32_t)(x1-x)*(y3-y);
      if((a>=0 && b>=0 && c>=0) || (a<=0 && b<=0 && c<=0)) {
        OLED_DrawPoint(dev, i, j, color);
      }
    }
  }
}

void OLED_DrawCircle(oled_device_t *dev, uint8_t x, uint8_t y, uint8_t r, oled_color_t color) {
  int16_t a = 0, b = r, di = 3 - (r << 1);
  while (a <= b) {
    OLED_DrawPoint(dev, x - b, y - a, color);
    OLED_DrawPoint(dev, x + b, y - a, color);
    OLED_DrawPoint(dev, x - a, y + b, color);
    OLED_DrawPoint(dev, x - b, y - a, color);
    OLED_DrawPoint(dev, x - a, y - b, color);
    OLED_DrawPoint(dev, x + b, y + a, color);
    OLED_DrawPoint(dev, x + a, y - b, color);
    OLED_DrawPoint(dev, x + a, y + b, color);
    OLED_DrawPoint(dev, x - b, y + a, color);
    a++;
    if (di < 0) di += 4 * a + 6;
    else {
      di += 10 + 4 * (a - b);
      b--;
    }
    OLED_DrawPoint(dev, x + a, y + b, color);
  }
}

void OLED_DrawFilledCircle(oled_device_t *dev, uint8_t x, uint8_t y, uint8_t r, oled_color_t color) {
  int16_t a = 0, b = r, di = 3 - (r << 1);
  while (a <= b) {
    for (int16_t i = x - b; i <= x + b; i++) {
      OLED_DrawPoint(dev, i, y + a, color);
      OLED_DrawPoint(dev, i, y - a, color);
    }
    for (int16_t i = x - a; i <= x + a; i++) {
      OLED_DrawPoint(dev, i, y + b, color);
      OLED_DrawPoint(dev, i, y - b, color);
    }
    a++;
    if (di < 0) di += 4 * a + 6;
    else {
      di += 10 + 4 * (a - b);
      b--;
    }
  }
}

void OLED_DrawEllipse(oled_device_t *dev, uint8_t x, uint8_t y, uint8_t a, uint8_t b, oled_color_t color) {
  int xpos = 0, ypos = b;
  int a2 = a * a, b2 = b * b;
  long d = b2 + a2 * (0.25 - b);
  while (a2 * ypos > b2 * xpos) {
    OLED_DrawPoint(dev, x + xpos, y + ypos, color);
    OLED_DrawPoint(dev, x - xpos, y + ypos, color);
    OLED_DrawPoint(dev, x + xpos, y - ypos, color);
    OLED_DrawPoint(dev, x - xpos, y - ypos, color);
    if (d < 0) {
      d = d + b2 * ((xpos << 1) + 3);
      xpos += 1;
    } else {
      d = d + b2 * ((xpos << 1) + 3) + a2 * (-(ypos << 1) + 2);
      xpos += 1, ypos -= 1;
    }
  }
  d = b2 * (xpos + 0.5) * (xpos + 0.5) + a2 * (ypos - 1) * (ypos - 1) - a2 * b2;
  while (ypos > 0) {
    OLED_DrawPoint(dev, x + xpos, y + ypos, color);
    OLED_DrawPoint(dev, x - xpos, y + ypos, color);
    OLED_DrawPoint(dev, x + xpos, y - ypos, color);
    OLED_DrawPoint(dev, x - xpos, y - ypos, color);
    if (d < 0) {
      d = d + b2 * ((xpos << 1) + 2) + a2 * (-(ypos << 1) + 3);
      xpos += 1, ypos -= 1;
    } else {
      d = d + a2 * (-(ypos << 1) + 3);
      ypos -= 1;
    }
  }
}

static void OLED_SetBlock(oled_device_t *dev, uint8_t x, uint8_t y,
                          const uint8_t *data, uint8_t w, uint8_t h,
                          oled_color_t color) {
  for (uint8_t i = 0; i < w; i++) {
    for (uint8_t j = 0; j < h; j++) {
      uint16_t byte_idx = i + (j / 8) * (uint16_t)w;
      uint8_t bit_val = (data[byte_idx] >> (j % 8)) & 0x01;
      if (color == OLED_COLOR_REVERSED) bit_val = !bit_val;
      OLED_DrawPoint(dev, x + i, y + j, bit_val ? OLED_COLOR_NORMAL : OLED_COLOR_REVERSED);
    }
  }
}

void OLED_DrawImage(oled_device_t *dev, uint8_t x, uint8_t y, const Image *img, oled_color_t color) {
  if (!dev || !img) return;
  OLED_SetBlock(dev, x, y, img->data, img->w, img->h, color);
}

#endif // OLED_CFG_USE_GRAPHICS

#if OLED_CFG_USE_FONTS

void OLED_PrintASCIIChar(oled_device_t *dev, uint8_t x, uint8_t y, char ch, const ASCIIFont *font, oled_color_t color) {
  if (!dev || !font) return;
  uint32_t char_size = ((font->h + 7) / 8) * font->w;
  const uint8_t *char_data = &font->chars[(ch - ' ') * char_size];
  OLED_SetBlock(dev, x, y, char_data, font->w, font->h, color);
}

void OLED_PrintASCIIString(oled_device_t *dev, uint8_t x, uint8_t y, char *str, const ASCIIFont *font, oled_color_t color) {
  if (!dev || !str || !font) return;
  while (*str) {
    OLED_PrintASCIIChar(dev, x, y, *str, font, color);
    x += font->w;
    str++;
  }
}

static uint8_t _OLED_GetUTF8Len(char *string) {
  if ((string[0] & 0x80) == 0x00) return 1;
  else if ((string[0] & 0xE0) == 0xC0) return 2;
  else if ((string[0] & 0xF0) == 0xE0) return 3;
  else if ((string[0] & 0xF8) == 0xF0) return 4;
  return 0;
}

void OLED_PrintString(oled_device_t *dev, uint8_t x, uint8_t y, char *str, const Font *font, oled_color_t color) {
  if (!dev || !str || !font) return;
  uint16_t i = 0;
  uint16_t one_char_bytes = (((font->h + 7) / 8) * font->w) + 4;
  while (str[i]) {
    uint8_t utf8Len = _OLED_GetUTF8Len(str + i);
    if (utf8Len == 0) break;
    bool found = false;
    for (uint8_t j = 0; j < font->len; j++) {
      const uint8_t *head = &font->chars[j * one_char_bytes];
      if (memcmp(str + i, head, utf8Len) == 0) {
        OLED_SetBlock(dev, x, y, head + 4, font->w, font->h, color);
        x += font->w;
        i += utf8Len;
        found = true;
        break;
      }
    }
    if (!found) {
      if (utf8Len == 1 && font->ascii) {
        OLED_PrintASCIIChar(dev, x, y, str[i], font->ascii, color);
        x += font->ascii->w;
        i += 1;
      } else {
        i += utf8Len;
      }
    }
  }
}

#endif // OLED_CFG_USE_FONTS
