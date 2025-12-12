/*
 * stm32_lcd_driver.h
 *
 *  Created on: Dec 8, 2025
 *      Author: Antigravity
 */

#ifndef BSP_DEVICE_DRIVER_PLATFORM_STM32_DEVICE_STM32_LCD_DRIVER_H_
#define BSP_DEVICE_DRIVER_PLATFORM_STM32_DEVICE_STM32_LCD_DRIVER_H_

#include "lcd_driver.h"
#include "stm32h7xx_hal.h"

// STM32 LCD driver structure
typedef struct stm32_lcd_driver {
  lcd_driver_t base;         // Base class
  LTDC_HandleTypeDef *hltdc; // LTDC Handle
} stm32_lcd_driver_t;

// Public constructor/init function
//lcd_driver_t *stm32_lcd_driver_create(LTDC_HandleTypeDef *hltdc, uint16_t width, uint16_t height);
lcd_driver_t *stm32_lcd_driver_create(LTDC_HandleTypeDef *hltdc);

#endif /* BSP_DEVICE_DRIVER_PLATFORM_STM32_DEVICE_STM32_LCD_DRIVER_H_ */
