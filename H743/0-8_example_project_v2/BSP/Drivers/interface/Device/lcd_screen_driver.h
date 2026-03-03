/*
 * screen_driver.h
 *
 *  Created on: Feb 26, 2026
 *      Author: 12114
 */

#ifndef DRIVERS_INTERFACE_DEVICE_LCD_SCREEN_DRIVER_H_
#define DRIVERS_INTERFACE_DEVICE_LCD_SCREEN_DRIVER_H_

#include <stdbool.h>
#include <stdint.h>

/* 前置声明 */
typedef struct lcd_driver_t lcd_driver_t;

typedef enum { HORIZONTAL = 0, VERTICAL = 1 } disp_direction_t;

typedef enum {
  LCD_PIXEL_RGB565 = 2,  // 2 bytes per pixel
  LCD_PIXEL_RGB888 = 3,  // 3 bytes per pixel
  LCD_PIXEL_ARGB8888 = 4 // 4 bytes per pixel
} pixel_format_t;

/**
 * @brief LCD 信息结构体
 */
typedef struct {
  uint16_t width;
  uint16_t height;
  pixel_format_t format;
  disp_direction_t dir;
  void *buffer_addr; // 使用 void* 更通用，因为可能是 SDRAM 地址
} lcd_screen_info_t;

/**
 * @brief LCD 驱动操作接口 V-Table
 */
typedef struct {
  /* 初始化，不再需要传入 info，直接使用 self->info */
  int (*init)(lcd_driver_t *self);

  /* 缓冲区管理 */
  int (*set_buffer)(lcd_driver_t *self, void *buffer);
  void* (*get_act_buffer)(lcd_driver_t *self);

  /* 控制指令 */
  int (*set_dir)(lcd_driver_t *self, disp_direction_t direction);
  disp_direction_t (*get_act_dir)(lcd_driver_t *self);
  int (*display_on)(lcd_driver_t *self);
  int (*display_off)(lcd_driver_t *self);

  /* 绘图指令 */
  uint32_t (*read_point)(lcd_driver_t *self, uint16_t x, uint16_t y);
  int (*draw_point)(lcd_driver_t *self, uint16_t x, uint16_t y, uint32_t color);

  /* 区域填充 (纯色) - 建议使用 GPU外设加速 */
  int (*fill_rect)(lcd_driver_t *self, uint16_t x, uint16_t y, uint16_t w,
                   uint16_t h, uint32_t color);

  /* 图片绘制 (位图) - 建议使用 GPU外设加速 */
  int (*draw_bitmap)(lcd_driver_t *self, uint16_t x, uint16_t y, uint16_t w,
                     uint16_t h, const void *bitmap);
} lcd_driver_ops_t;

/**
 * @brief LCD 驱动基类
 */
typedef struct lcd_driver_t {
  const lcd_driver_ops_t *ops;
  lcd_screen_info_t info;
} lcd_driver_t;

#define LCD_INIT(self) (self)->ops->init(self)
#define LCD_SET_BUFFER(self, buffer) (self)->ops->set_buffer(self, buffer)
#define LCD_SET_DIR(self, dir) (self)->ops->set_dir(self, dir)
#define LCD_DISPLAY_ON(self) (self)->ops->display_on(self)
#define LCD_DISPLAY_OFF(self) (self)->ops->display_off(self)
#define LCD_READ_POINT(self, x, y) (self)->ops->read_point(self, x, y)
#define LCD_DRAW_POINT(self, x, y, color)                                      \
  (self)->ops->draw_point(self, x, y, color)
#define LCD_FILL(self, x, y, w, h, color)                                      \
  (self)->ops->fill_rect(self, x, y, w, h, color)
#define LCD_DRAW_BITMAP(self, x, y, w, h, bitmap)                              \
  (self)->ops->draw_bitmap(self, x, y, w, h, bitmap)

#endif /* DRIVERS_INTERFACE_DEVICE_LCD_SCREEN_DRIVER_H_ */
