#include "lcd_hal.h"
#include "sys.h"
#include <stdlib.h>

void lcd_hal_init(lcd_hal_t *self, lcd_driver_t *driver) {
  if (!self || !driver) {
    return;
  }
  self->driver = driver;
}

lcd_hal_t *lcd_hal_create(lcd_driver_t *driver, uint16_t *draw_buffer,
                          uint16_t *display_buffer) {
  lcd_hal_t *self = (lcd_hal_t *)sys_malloc(LCD_MEMSOURCE, sizeof(lcd_hal_t));
  if (self) {
    if (driver && driver->ops && driver->ops->set_drawbuf)
      LCD_SET_DRAWBUF(driver, draw_buffer);
    if (driver && driver->ops && driver->ops->set_displaybuf)
      LCD_SET_DISPLAYBUF(driver, display_buffer);
    lcd_hal_init(self, driver);
  }
  return self;
}

void lcd_hal_destroy(lcd_hal_t *self) {
  if (!self) {
    return;
  }
  sys_free(LCD_MEMSOURCE, self);
}
