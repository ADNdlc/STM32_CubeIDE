/*
 * stm32_qspi_driver.h
 *
 *  Created on: Feb 16, 2026
 *      Author: Antigravity
 */

#ifndef BSP_DRIVERS_PLATFORM_STM32_QSPI_DRIVER_H_
#define BSP_DRIVERS_PLATFORM_STM32_QSPI_DRIVER_H_

#include "qspi_driver.h"
#include "stm32h7xx_hal.h"

// STM32 QSPI 驱动派生类
typedef struct {
  qspi_driver_t base;
  QSPI_HandleTypeDef *hqspi;
} stm32_qspi_driver_t;

// 工厂方法
qspi_driver_t *stm32_qspi_driver_create(QSPI_HandleTypeDef *hqspi);

#endif /* BSP_DRIVERS_PLATFORM_STM32_QSPI_DRIVER_H_ */
