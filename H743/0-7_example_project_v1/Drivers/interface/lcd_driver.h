/*
 * lcd_driver.h
 *
 *  Created on: Dec 8, 2025
 *      Author: Antigravity
 */

#ifndef BSP_DEVICE_DRIVER_INTERFACE_LCD_DRIVER_H_
#define BSP_DEVICE_DRIVER_INTERFACE_LCD_DRIVER_H_

#include <stddef.h>
#include <stdint.h>

// Forward declaration
typedef struct lcd_driver_t lcd_driver_t;

// 定义显示方向
typedef enum
{
  LCD_ORIENTATION_PORTRAIT = 0,
  LCD_ORIENTATION_LANDSCAPE
} lcd_orientation_t;

// 定义依赖硬件的操作
typedef struct
{
  // 基本绘制
  void (*draw_point)(lcd_driver_t *self, uint16_t x, uint16_t y,
                     uint32_t color);
  uint32_t (*read_point)(lcd_driver_t *self, uint16_t x, uint16_t y);
  void (*fill_rect)(lcd_driver_t *self, uint16_t x, uint16_t y, uint16_t w,
                    uint16_t h, uint32_t color);

  // bitmap填充(一般使用dma2d等硬件加速来实现)
  // pSrc 必须与LCD设备支持的色彩格式相同 (e.g. RGB565)
  void (*draw_bitmap)(lcd_driver_t *self, uint16_t x, uint16_t y, uint16_t w,
                      uint16_t h, const void *pSrc);
  // 缓冲区拷贝(将已绘制的数据块拷贝到绘制区)
  void (*copy_buffer)(lcd_driver_t *self,    // lcd对象 (目标)
                      void *pSrc,            // 源
                      uint32_t xSize,        // 宽度
                      uint32_t ySize,        // 高度
                      uint32_t OffLineSrc,   // 源偏移
                      uint32_t OffLineDst,   // 目标偏移
                      uint32_t PixelFormat); // 像素格式

  // 控制
  void (*set_orientation)(lcd_driver_t *self,
                          lcd_orientation_t orientation);    // 设置显示方向
  void (*swap_buffer)(lcd_driver_t *self);                   // 交换活动缓冲区(如果使用双缓冲，请实现此接口)
  uint16_t *(*get_drawbuf)(lcd_driver_t *self);              // 获取绘制缓冲区
  uint16_t *(*get_displaybuf)(lcd_driver_t *self);           // 获取显示缓冲区
  void (*set_drawbuf)(lcd_driver_t *self, uint16_t *buffer); // 设置绘制缓冲区
  void (*set_displaybuf)(lcd_driver_t *self,
                         uint16_t *buffer); // 设置显示缓冲区
} lcd_driver_ops_t;

// 定义依赖硬件的数据
struct lcd_driver_t
{
  const lcd_driver_ops_t *ops; // 操作
  // 设备信息
  uint16_t width;  // 显示宽度
  uint16_t height; // 显示高度
  // 显存地址指针(rgb565)
  uint16_t *draw_buffer;
  uint16_t *display_buffer;
  lcd_orientation_t orientation; // 显示方向
};

// 辅助宏

#define LCD_DRAW_POINT(driver, x, y, color) \
  (driver)->ops->draw_point(driver, x, y, color)

#define LCD_READ_POINT(driver, x, y) (driver)->ops->read_point(driver, x, y)

#define LCD_FILL_RECT(driver, x, y, w, h, color) \
  (driver)->ops->fill_rect(driver, x, y, w, h, color)

#define LCD_DRAW_BITMAP(driver, x, y, w, h, pSrc) \
  (driver)->ops->draw_bitmap(driver, x, y, w, h, pSrc)

#define LCD_SET_ORIENTATION(driver, orientation) \
  (driver)->ops->set_orientation(driver, orientation)

#define LCD_SWAP_BUFFER(driver) (driver)->ops->swap_buffer(driver)
#define LCD_GET_DRAWBUF(driver) (driver)->ops->get_drawbuf(driver)
#define LCD_GET_DISPLAYBUF(driver) (driver)->ops->get_displaybuf(driver)
#define LCD_SET_DRAWBUF(driver, buffer) \
  (driver)->ops->set_drawbuf(driver, buffer)
#define LCD_SET_DISPLAYBUF(driver, buffer) \
  (driver)->ops->set_displaybuf(driver, buffer)

#endif /* BSP_DEVICE_DRIVER_INTERFACE_LCD_DRIVER_H_ */
