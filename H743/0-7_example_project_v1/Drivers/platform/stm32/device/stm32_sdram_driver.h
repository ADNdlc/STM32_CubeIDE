/*
 * stm32_sdram_driver.h
 *
 *  Created on: Dec 8, 2025
 *      Author: Antigravity
 */

#ifndef BSP_DEVICE_DRIVER_PLATFORM_STM32_DEVICE_STM32_SDRAM_DRIVER_H_
#define BSP_DEVICE_DRIVER_PLATFORM_STM32_DEVICE_STM32_SDRAM_DRIVER_H_

#include "sdram_driver.h"
#include "stm32h7xx_hal.h"

// STM32 SDRAM driver structure
typedef struct {
  sdram_driver_t base;         // Base class
  SDRAM_HandleTypeDef *hsdram; // SDRAM Handle
} stm32_sdram_driver_t;

// Public constructor/init function
sdram_driver_t *stm32_sdram_create(SDRAM_HandleTypeDef *hsdram);

#endif /* BSP_DEVICE_DRIVER_PLATFORM_STM32_DEVICE_STM32_SDRAM_DRIVER_H_ */
