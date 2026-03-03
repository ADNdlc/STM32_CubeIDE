/*
 * stm32_ltdc_driver.h
 *
 *  Created on: Mar 1, 2026
 *      Author: Antigravity
 */

#ifndef BSP_DRIVERS_PLATFORM_STM32_DEVICE_STM32_LTDC_DRIVER_H_
#define BSP_DRIVERS_PLATFORM_STM32_DEVICE_STM32_LTDC_DRIVER_H_

#include "lcd_screen_driver.h"
#include "stm32h7xx_hal.h"

typedef struct {
  LTDC_HandleTypeDef *hltdc;
  DMA2D_HandleTypeDef *hdma2d;
  uint8_t layer;
  uint8_t bl_gpio_id;
} stm32_ltdc_config_t;

/**
 * @brief STM32 LTDC 派生类
 */
typedef struct {
  lcd_driver_t base;           // 继承基类
  LTDC_HandleTypeDef *hltdc;   // LTDC 句柄
  DMA2D_HandleTypeDef *hdma2d; // DMA2D 句柄 (关键！)
  uint8_t layer_index;         // 操作的层索引 (0 或 1)
  uint8_t bl_gpio_id;          // 背光 GPIO ID
} stm32_ltdc_driver_t;

/**
 * @brief 创建 STM32 LTDC 驱动实例
 * @param hltdc HAL LTDC 句柄
 * @param hdma2d HAL DMA2D 句柄 (可为 NULL，但建议提供)
 */
lcd_driver_t *stm32_ltdc_driver_create(stm32_ltdc_config_t *congfig,
                                       lcd_screen_info_t info);

#endif /* BSP_DRIVERS_PLATFORM_STM32_DEVICE_STM32_LTDC_DRIVER_H_ */
