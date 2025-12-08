#include "lcd_hal.h"
#include <stdlib.h>

// 实现基本绘图函数
void _lcd_hal_draw_point(lcd_driver_t *self, uint16_t x, uint16_t y, uint32_t color)
{
    if (self && self->ops && self->ops->draw_point)
    {
        LCD_DRAW_POINT(self, x, y, color);
    }
}

uint32_t _lcd_hal_read_point(lcd_driver_t *self, uint16_t x, uint16_t y)
{
    if (self && self->ops && self->ops->read_point)
    {
        return LCD_READ_POINT(self, x, y);
    }
    return 0;
}

void _lcd_hal_fill_rect(lcd_driver_t *self, uint16_t x, uint16_t y, uint16_t w,
                        uint16_t h, uint32_t color)
{
    if (self && self->ops && self->ops->fill_rect)
    {
        LCD_FILL_RECT(self, x, y, w, h, color);
    }
}

void _lcd_hal_draw_bitmap(lcd_driver_t *self, uint16_t x, uint16_t y, uint16_t w,
                          uint16_t h, const void *pSrc)
{
    if (self && self->ops && self->ops->draw_bitmap)
    {
        LCD_DRAW_BITMAP(self, x, y, w, h, pSrc);
    }
}

void _lcd_hal_copy_buffer(lcd_driver_t *self, void *pDst, uint32_t xSize,
                          uint32_t ySize, uint32_t OffLineSrc, uint32_t OffLineDst,
                          uint32_t PixelFormat)
{
    if (self && self->ops && self->ops->copy_buffer)
    {
        self->ops->copy_buffer(self, pDst, xSize, ySize, OffLineSrc, OffLineDst, PixelFormat);
    }
}

void _lcd_hal_set_orientation(lcd_driver_t *self, lcd_orientation_t orientation)
{
    if (self && self->ops && self->ops->set_orientation)
    {
        LCD_SET_ORIENTATION(self, orientation);
    }
}

void _lcd_hal_swap_buffer(lcd_driver_t *self)
{
    if (self && self->ops && self->ops->swap_buffer)
    {
        LCD_SWAP_BUFFER(self);
    }
}

uint16_t *_lcd_hal_get_drawbuf(lcd_driver_t *self)
{
    if (self && self->ops && self->ops->get_drawbuf)
    {
        return LCD_GET_DRAWBUF(self);
    }
    return NULL;
}

uint16_t *_lcd_hal_get_displaybuf(lcd_driver_t *self)
{
    if (self && self->ops && self->ops->get_displaybuf)
    {
        return LCD_GET_DISPLAYBUF(self);
    }
    return NULL;
}

// lcd设备虚表
lcd_hal_vtable_t lcd_hal_vtable = {
    .base_vtable = {
        .draw_point = _lcd_hal_draw_point,
        .read_point = _lcd_hal_read_point,
        .fill_rect = _lcd_hal_fill_rect,
        .draw_bitmap = _lcd_hal_draw_bitmap,
        .copy_buffer = _lcd_hal_copy_buffer,
        .set_orientation = _lcd_hal_set_orientation,
        .swap_buffer = _lcd_hal_swap_buffer,
        .get_drawbuf = _lcd_hal_get_drawbuf,
        .get_displaybuf = _lcd_hal_get_displaybuf,
    }};

void lcd_hal_init(lcd_hal_t *self, lcd_driver_t *driver)
{
    if (!self || !driver)
    {
        return;
    }

    self->vtable = &lcd_hal_vtable;
    // 复制驱动信息
    self->base.ops = driver->ops;
    self->base.width = driver->width;
    self->base.height = driver->height;
    self->base.draw_buffer = driver->draw_buffer;
    self->base.display_buffer = driver->display_buffer;
    self->base.orientation = driver->orientation;
}

lcd_hal_t *lcd_hal_create(lcd_driver_t *driver)
{
    lcd_hal_t *self = (lcd_hal_t *)malloc(sizeof(lcd_hal_t));
    if (self)
    {
        lcd_hal_init(self, driver);
    }
    return self;
}

void lcd_hal_destroy(lcd_hal_t *self)
{
    if (!self)
    {
        return;
    }
    free(self);
}