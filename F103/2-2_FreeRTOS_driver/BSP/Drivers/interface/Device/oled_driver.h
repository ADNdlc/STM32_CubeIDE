/*
 * oled_driver.h
 *
 *  Created on: Mar 2, 2026
 *      Author: 12114
 */

#ifndef DRIVERS_INTERFACE_DEVICE_OLED_OLED_DRIVER_H_
#define DRIVERS_INTERFACE_DEVICE_OLED_OLED_DRIVER_H_

#include <stdbool.h>
#include <stdint.h>
#include "oled_font.h"


/* 前向声明 */
typedef struct oled_device_t oled_device_t;
typedef struct oled_bus_t oled_bus_t;
typedef struct oled_chip_ops_t oled_chip_ops_t;
/* ========================================= */
/* 1. 总线适配器接口 (Transport Layer)       */
/* ========================================= */
typedef enum { OLED_CMD = 0, OLED_DATA = 1 } oled_data_type_t;

struct oled_bus_t{
  // 基础发送接口
  int (*write)(oled_bus_t *self, oled_data_type_t type, const uint8_t *data, uint32_t len);
};
/* ========================================= */
/* 2. 芯片操作接口 (Chip Ops Layer)          */
/* ========================================= */
struct oled_chip_ops_t{
  int (*init)(oled_device_t *dev);
  int (*set_cursor)(oled_device_t *dev, uint8_t page, uint8_t col);
  int (*set_contrast)(oled_device_t *dev, uint8_t contrast);
  int (*display_on)(oled_device_t *dev);
  int (*display_off)(oled_device_t *dev);
};

/* ========================================= */
/* 3. 设备实体定义 (Device Layer)            */
/* ========================================= */

typedef struct oled_device_t {
  const oled_chip_ops_t *ops; // 绑定的芯片驱动 (如 SSD1306)
  oled_bus_t *bus;            // 绑定的通信总线 (如 I2C)

  uint16_t width;     // 宽度 (如 128)
  uint16_t height;    // 高度 (如 64 或 32)
  uint8_t col_offset; // 列偏移 (SSD1306通常为0，SH1106为2)
  uint8_t *buffer; // 缓冲区
}oled_device_t;

typedef enum {
  OLED_COLOR_NORMAL = 0, // 正常模式 黑底白字
  OLED_COLOR_REVERSED    // 反色模式 白底黑字
} oled_color_t;

/* ========================================= */
/* 4. 配置宏 (Feature Isolation)             */
/* ========================================= */
#define OLED_CFG_USE_GRAPHICS 1
#define OLED_CFG_USE_FONTS    1

/* ========================================= */
/* 5. 图形与刷新 API                         */
/* ========================================= */
void OLED_Init(oled_device_t *dev);
void OLED_DisPlay_On(oled_device_t *dev);
void OLED_DisPlay_Off(oled_device_t *dev);

void OLED_ReFresh(oled_device_t *dev);
void OLED_Clear(oled_device_t *dev, oled_color_t mode);

#if OLED_CFG_USE_GRAPHICS
void OLED_DrawPoint(oled_device_t *dev, uint8_t x, uint8_t y, oled_color_t mode);
void OLED_DrawLine(oled_device_t *dev, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, oled_color_t color);
void OLED_DrawRectangle(oled_device_t *dev, uint8_t x, uint8_t y, uint8_t w, uint8_t h, oled_color_t color);
void OLED_DrawFilledRectangle(oled_device_t *dev, uint8_t x, uint8_t y, uint8_t w, uint8_t h, oled_color_t color);
void OLED_DrawTriangle(oled_device_t *dev, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, oled_color_t color);
void OLED_DrawFilledTriangle(oled_device_t *dev, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, oled_color_t color);
void OLED_DrawCircle(oled_device_t *dev, uint8_t x, uint8_t y, uint8_t r, oled_color_t color);
void OLED_DrawFilledCircle(oled_device_t *dev, uint8_t x, uint8_t y, uint8_t r, oled_color_t color);
void OLED_DrawEllipse(oled_device_t *dev, uint8_t x, uint8_t y, uint8_t a, uint8_t b, oled_color_t color);
void OLED_DrawImage(oled_device_t *dev, uint8_t x, uint8_t y, const Image *img, oled_color_t color);
#endif

#if OLED_CFG_USE_FONTS
void OLED_PrintASCIIChar(oled_device_t *dev, uint8_t x, uint8_t y, char ch, const ASCIIFont *font, oled_color_t color);
void OLED_PrintASCIIString(oled_device_t *dev, uint8_t x, uint8_t y, char *str, const ASCIIFont *font, oled_color_t color);
void OLED_PrintString(oled_device_t *dev, uint8_t x, uint8_t y, char *str, const Font *font, oled_color_t color);
#endif

#endif /* DRIVERS_INTERFACE_DEVICE_OLED_OLED_DRIVER_H_ */
