/*
 * stm32_spi_driver.h
 *
 *  Created on: Feb 16, 2026
 *      Author: Antigravity
 */

#ifndef BSP_DRIVERS_PLATFORM_STM32_SPI_DRIVER_H_
#define BSP_DRIVERS_PLATFORM_STM32_SPI_DRIVER_H_

#include "spi_driver.h"
#include "HAL_include.h"

// STM32 SPI 驱动派生类
typedef struct {
  spi_driver_t base;
  SPI_HandleTypeDef *hspi;
} stm32_spi_driver_t;

// 工厂方法
spi_driver_t *stm32_spi_driver_create(SPI_HandleTypeDef *hspi);

// 为工厂提供的便捷宏
#define STM32_SPI_DRIVER_CREATE(hspi) stm32_spi_driver_create(hspi)

#endif /* BSP_DRIVERS_PLATFORM_STM32_SPI_DRIVER_H_ */
