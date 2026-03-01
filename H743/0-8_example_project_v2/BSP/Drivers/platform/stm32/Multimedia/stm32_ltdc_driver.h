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

/**
 * @brief STM32 LTDC 驱动私有结构体
 */
typedef struct {
  lcd_driver_t base;           // 基类
  LTDC_HandleTypeDef *hltdc;   // LTDC 句柄
  DMA2D_HandleTypeDef *hdma2d; // DMA2D 句柄
  uint32_t frame_buffer;       // 显存起始地址
} stm32_ltdc_driver_t;

/**
 * @brief 创建 STM32 LTDC 驱动实例
 * @param hltdc HAL LTDC 句柄
 * @param hdma2d HAL DMA2D 句柄
 * @param fb_addr 显存起始地址 (通常位于 SDRAM)
 * @return lcd_driver_t* 驱动实例指针
 */
lcd_driver_t *stm32_ltdc_driver_create(LTDC_HandleTypeDef *hltdc,
                                       DMA2D_HandleTypeDef *hdma2d,
                                       uint32_t fb_addr);

#endif /* BSP_DRIVERS_PLATFORM_STM32_DEVICE_STM32_LTDC_DRIVER_H_ */
