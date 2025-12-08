/*
 * lcd_factory.c
 *
 *  Created on: Dec 8, 2025
 *      Author: Antigravity
 */

#include "lcd_factory.h"
#include "device_mapping.h"
#include "stm32_lcd_driver.h"
#include <stddef.h>

static lcd_driver_t *lcd_drivers[LCD_MAX_DEVICES] = {NULL};

lcd_driver_t *lcd_driver_get(lcd_device_id_t id) {
  if (id >= LCD_MAX_DEVICES) {
    return NULL;
  }
  if (lcd_drivers[id] == NULL) {
    const lcd_mapping_t *mapping = &lcd_mappings[id];
    lcd_drivers[id] = stm32_lcd_create(mapping->hltdc);
  }
  return lcd_drivers[id];
}
