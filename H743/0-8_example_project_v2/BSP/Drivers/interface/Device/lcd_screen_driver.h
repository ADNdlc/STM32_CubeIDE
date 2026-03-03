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

/* 定义异步回调函数类型 */
typedef void (*lcd_async_cb_t)(void *user_data);

typedef enum { HORIZONTAL = 0, VERTICAL = 1 } disp_direction_t;

typedef enum {
  LCD_PIXEL_RGB565 = 2,  // 2 bytes per pixel
  LCD_PIXEL_ARGB8888 = 4 // 4 bytes per pixel
} pixel_format_t;

// 获取像素字节数
static inline uint8_t get_pixel_bytes(pixel_format_t fmt) {
  if (fmt == LCD_PIXEL_RGB565)
    return 2;
  if (fmt == LCD_PIXEL_ARGB8888)
    return 4;
  return 4; // 默认
}

/**
 * @brief LCD 信息结构体
 */
typedef struct {
  uint16_t width;
  uint16_t height;
  pixel_format_t format;
  disp_direction_t dir;
  void *buffer_addr; // 前台缓冲区 (正在显示)
  void *back_buffer; // 后台缓冲区 (正在绘制)
} lcd_screen_info_t;

/**
 * @brief LCD 驱动操作接口 V-Table
 */
typedef struct {
  /* 初始化，不再需要传入 info，直接使用 self->info */
  int (*init)(lcd_driver_t *self);

  /* 缓冲区管理 */
  int (*set_buffer)(lcd_driver_t *self, void *buf1, void *buf2);
  void *(*get_act_buffer)(lcd_driver_t *self);
  void *(*get_back_buffer)(lcd_driver_t *self);

  /* 控制指令 */
  int (*set_dir)(lcd_driver_t *self, disp_direction_t direction);
  disp_direction_t (*get_act_dir)(lcd_driver_t *self);
  int (*display_on)(lcd_driver_t *self);
  int (*display_off)(lcd_driver_t *self);

  /* 双缓冲控制 */
  int (*swap_buffer)(lcd_driver_t *self);
  int (*wait_swap)(lcd_driver_t *self);

  /* 绘图指令 */
  uint32_t (*read_point)(lcd_driver_t *self, uint16_t x, uint16_t y);
  int (*draw_point)(lcd_driver_t *self, uint16_t x, uint16_t y, uint32_t color);

  /* 区域填充 (纯色) - 建议使用 GPU外设加速 */
  int (*fill_rect)(lcd_driver_t *self, uint16_t x, uint16_t y, uint16_t w,
                   uint16_t h, uint32_t color);

  /* 图片绘制 (位图) - 建议使用 GPU外设加速 */
  int (*draw_bitmap)(lcd_driver_t *self, uint16_t x, uint16_t y, uint16_t w,
                     uint16_t h, const void *bitmap);

  /* 异步回调注册接口 */
  int (*set_swap_cb)(lcd_driver_t *self, lcd_async_cb_t cb, void *user_data);
  int (*set_fill_cb)(lcd_driver_t *self, lcd_async_cb_t cb, void *user_data);
} lcd_driver_ops_t;

/**
 * @brief LCD 驱动基类
 */
typedef struct lcd_driver_t {
  const lcd_driver_ops_t *ops;
  lcd_screen_info_t info;
  /* 异步回调上下文 (由实现层的硬件中断直接调用) */
  lcd_async_cb_t swap_done_cb;
  void *swap_cb_data;
  lcd_async_cb_t fill_done_cb;
  void *fill_cb_data;
} lcd_driver_t;

#define LCD_INIT(self) (self)->ops->init(self)
#define LCD_SET_BUFFER(self, buffer, type)                                     \
  (self)->ops->set_buffer(self, buffer, type)
#define LCD_GET_ACT_BUFFER(self) (self)->ops->get_act_buffer(self)
#define LCD_GET_BACK_BUFFER(self) (self)->ops->get_back_buffer(self)
#define LCD_SET_DIR(self, dir) (self)->ops->set_dir(self, dir)
#define LCD_GET_ACT_DIR(self) (self)->ops->get_act_dir(self)
#define LCD_DISPLAY_ON(self) (self)->ops->display_on(self)
#define LCD_DISPLAY_OFF(self) (self)->ops->display_off(self)
#define LCD_SWAP_BUFFER(self) (self)->ops->swap_buffer(self)
#define LCD_WAIT_SWAP(self) (self)->ops->wait_swap(self)
#define LCD_READ_POINT(self, x, y) (self)->ops->read_point(self, x, y)
#define LCD_DRAW_POINT(self, x, y, color)                                      \
  (self)->ops->draw_point(self, x, y, color)
#define LCD_FILL(self, x, y, w, h, color)                                      \
  (self)->ops->fill_rect(self, x, y, w, h, color)
#define LCD_DRAW_BITMAP(self, x, y, w, h, bitmap)                              \
  (self)->ops->draw_bitmap(self, x, y, w, h, bitmap)
#define LCD_SET_SWAP_CB(self, cb, data) (self)->ops->set_swap_cb(self, cb, data)
#define LCD_SET_FILL_CB(self, cb, data) (self)->ops->set_fill_cb(self, cb, data)

#endif /* DRIVERS_INTERFACE_DEVICE_LCD_SCREEN_DRIVER_H_ */
