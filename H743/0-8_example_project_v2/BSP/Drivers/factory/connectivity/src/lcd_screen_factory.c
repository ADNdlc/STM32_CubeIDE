/*
 * lcd_screen_factory.c
 *
 *  Created on: Mar 2, 2026
 *      Author: 12114
 */


#include "lcd_screen_factory.h"
#include <stddef.h>
#include <stdint.h>

#if (TARGET_PLATFORM == PLATFORM_STM32)
#include "ltdc.h"
#include "dma2d.h"
#include "Multimedia/stm32_ltdc_driver.h"
#endif

static lcd_driver_t *lcd_drivers[LCD_ID_MAX] = {NULL};

lcd_driver_t *lcd_screen_factory_creat(lcd_id_t id) {
  if (id >= LCD_ID_MAX) {
    return NULL;
  }

  if (lcd_drivers[id] == NULL) {
    const lcd_mapping_t *mapping = &lcd_mappings[id];
#if (TARGET_PLATFORM == PLATFORM_STM32)
    // 创建 ltdc lcd 驱动
    lcd_drivers[id] = stm32_ltdc_driver_create((stm32_ltdc_config_t*)mapping->resource, mapping->info);
#endif
  }

  return lcd_drivers[id];
}

