#ifndef LCD_LCD_HAL_H_
#define LCD_LCD_HAL_H_

#include "lcd_driver.h"
#include <stdint.h>

// 从哪里分配
#define LCD_MEMSOURCE SYS_MEM_INTERNAL

// 前向声明
typedef struct lcd_hal_t lcd_hal_t;

// 虚表定义
// HAL结构体定义
struct lcd_hal_t {
  lcd_driver_t *driver;
};

// 函数声明
void lcd_hal_init(lcd_hal_t *self, lcd_driver_t *driver);
lcd_hal_t *lcd_hal_create(lcd_driver_t *driver, uint16_t *draw_buffer,
                          uint16_t *display_buffer);
void lcd_hal_destroy(lcd_hal_t *self);

// 内联多态调用函数
static inline void lcd_hal_draw_point(lcd_hal_t *self, uint16_t x, uint16_t y,
                                      uint32_t color) {
  if (self && self->driver && self->driver->ops &&
      self->driver->ops->draw_point) {
    self->driver->ops->draw_point(self->driver, x, y, color);
  }
}

static inline uint32_t lcd_hal_read_point(lcd_hal_t *self, uint16_t x,
                                          uint16_t y) {
  if (self && self->driver && self->driver->ops &&
      self->driver->ops->read_point) {
    return self->driver->ops->read_point(self->driver, x, y);
  }
  return 0;
}

static inline void lcd_hal_fill_rect(lcd_hal_t *self, uint16_t x, uint16_t y,
                                     uint16_t w, uint16_t h, uint32_t color) {
  if (self && self->driver && self->driver->ops &&
      self->driver->ops->fill_rect) {
    self->driver->ops->fill_rect(self->driver, x, y, w, h, color);
  }
}

static inline void lcd_hal_draw_bitmap(lcd_hal_t *self, uint16_t x, uint16_t y,
                                       uint16_t w, uint16_t h,
                                       const void *pSrc) {
  if (self && self->driver && self->driver->ops &&
      self->driver->ops->draw_bitmap) {
    self->driver->ops->draw_bitmap(self->driver, x, y, w, h, pSrc);
  }
}

static inline void lcd_hal_copy_buffer(lcd_hal_t *self, void *pDst, void *pSrc,
                                       uint32_t xSize, uint32_t ySize,
                                       uint32_t OffLineSrc, uint32_t OffLineDst,
                                       uint32_t PixelFormat) {
  if (self && self->driver && self->driver->ops) {
    self->driver->ops->copy_buffer(pDst, pSrc, xSize, ySize, OffLineSrc,
                                   OffLineDst, PixelFormat);
  }
}

static inline void lcd_hal_set_orientation(lcd_hal_t *self,
                                           lcd_orientation_t orientation) {
  if (self && self->driver && self->driver->ops &&
      self->driver->ops->set_orientation) {
    self->driver->ops->set_orientation(self->driver, orientation);
  }
}

static inline void lcd_hal_swap_buffer(lcd_hal_t *self) {
  if (self && self->driver && self->driver->ops &&
      self->driver->ops->swap_buffer) {
    self->driver->ops->swap_buffer(self->driver);
  }
}

static inline uint16_t *lcd_hal_get_drawbuf(lcd_hal_t *self) {
  if (self && self->driver && self->driver->ops &&
      self->driver->ops->get_drawbuf) {
    return self->driver->ops->get_drawbuf(self->driver);
  }
  return 0;
}

static inline uint16_t *lcd_hal_get_displaybuf(lcd_hal_t *self) {
  if (self && self->driver && self->driver->ops &&
      self->driver->ops->get_displaybuf) {
    return self->driver->ops->get_displaybuf(self->driver);
  }
  return 0;
}

static inline void lcd_hal_set_drawbuf(lcd_hal_t *self, uint16_t *buffer) {
  if (self && self->driver && self->driver->ops &&
      self->driver->ops->set_drawbuf) {
    self->driver->ops->set_drawbuf(self->driver, buffer);
  }
}

static inline void lcd_hal_set_displaybuf(lcd_hal_t *self, uint16_t *buffer) {
  if (self && self->driver && self->driver->ops &&
      self->driver->ops->set_displaybuf) {
    self->driver->ops->set_displaybuf(self->driver, buffer);
  }
}

#endif /* LCD_LCD_HAL_H_ */
