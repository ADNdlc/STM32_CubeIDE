#ifndef LCD_LCD_HAL_H_
#define LCD_LCD_HAL_H_

#include "lcd_driver.h"
#include <stdint.h>

// 从哪里分配
#define LCD_MEMSOURCE SYS_MEM_INTERNAL

// 前向声明
typedef struct lcd_hal_t lcd_hal_t;

// 虚表定义
typedef struct {
  lcd_driver_ops_t base_vtable;
} lcd_hal_vtable_t;

// HAL结构体定义
struct lcd_hal_t {
  lcd_hal_vtable_t *vtable;
  // 依赖驱动
  lcd_driver_t base;
};

// 函数声明
void lcd_hal_init(lcd_hal_t *self, lcd_driver_t *driver);
lcd_hal_t *lcd_hal_create(lcd_driver_t *driver, uint16_t *draw_buffer,
                          uint16_t *display_buffer);
void lcd_hal_destroy(lcd_hal_t *self);

// 内联多态调用函数
static inline void lcd_hal_draw_point(lcd_hal_t *self, uint16_t x, uint16_t y,
                                      uint32_t color) {
  self->vtable->base_vtable.draw_point(&self->base, x, y, color);
}

static inline uint32_t lcd_hal_read_point(lcd_hal_t *self, uint16_t x,
                                          uint16_t y) {
  return self->vtable->base_vtable.read_point(&self->base, x, y);
}

static inline void lcd_hal_fill_rect(lcd_hal_t *self, uint16_t x, uint16_t y,
                                     uint16_t w, uint16_t h, uint32_t color) {
  self->vtable->base_vtable.fill_rect(&self->base, x, y, w, h, color);
}

static inline void lcd_hal_draw_bitmap(lcd_hal_t *self, uint16_t x, uint16_t y,
                                       uint16_t w, uint16_t h,
                                       const void *pSrc) {
  self->vtable->base_vtable.draw_bitmap(&self->base, x, y, w, h, pSrc);
}

static inline void lcd_hal_copy_buffer(lcd_hal_t *self, void *pDst,
                                       uint32_t xSize, uint32_t ySize,
                                       uint32_t OffLineSrc, uint32_t OffLineDst,
                                       uint32_t PixelFormat) {
  self->vtable->base_vtable.copy_buffer(&self->base, pDst, xSize, ySize,
                                        OffLineSrc, OffLineDst, PixelFormat);
}

static inline void lcd_hal_set_orientation(lcd_hal_t *self,
                                           lcd_orientation_t orientation) {
  self->vtable->base_vtable.set_orientation(&self->base, orientation);
}

static inline void lcd_hal_swap_buffer(lcd_hal_t *self) {
  self->vtable->base_vtable.swap_buffer(&self->base);
}

static inline uint16_t *lcd_hal_get_drawbuf(lcd_hal_t *self) {
  return self->vtable->base_vtable.get_drawbuf(&self->base);
}

static inline uint16_t *lcd_hal_get_displaybuf(lcd_hal_t *self) {
  return self->vtable->base_vtable.get_displaybuf(&self->base);
}

static inline void lcd_hal_set_drawbuf(lcd_hal_t *self, uint16_t *buffer) {
  self->vtable->base_vtable.set_drawbuf(&self->base, buffer);
}

static inline void lcd_hal_set_displaybuf(lcd_hal_t *self, uint16_t *buffer) {
  self->vtable->base_vtable.set_displaybuf(&self->base, buffer);
}

#endif /* LCD_LCD_HAL_H_ */
