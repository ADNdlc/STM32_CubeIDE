/*
 * lcd_factory.c
 *
 *  Created on: Dec 8, 2025
 *      Author: Antigravity
 */

#include "lcd_factory.h"
#include "device_mapping.h"
#include "factory_config.h"


#if (LCD_DRIVER_PLATFORM == PLATFORM_STM32)
#include "stm32_lcd_driver.h"
#endif

static lcd_driver_t *lcd_drivers[LCD_MAX_DEVICES] = {NULL};

//lcd_driver_t *lcd_driver_get(lcd_device_id_t id, uint16_t width, uint16_t height) {
lcd_driver_t *lcd_driver_get(lcd_device_id_t id) {
  if (id >= LCD_MAX_DEVICES) {
    return NULL;
  }
  if (lcd_drivers[id] == NULL) {
    const lcd_mapping_t *mapping = &lcd_mappings[id];

// 根据平台配置创建相应的LCD驱动实例
#if (LCD_DRIVER_PLATFORM == PLATFORM_STM32)
//    lcd_drivers[id] = stm32_lcd_driver_create(mapping->hltdc, width, height);
    lcd_drivers[id] = stm32_lcd_driver_create(mapping->hltdc);
#else
#error "未定义LCD_DRIVER_PLATFORM或平台不支持"
#endif

  }
  return lcd_drivers[id];
}
