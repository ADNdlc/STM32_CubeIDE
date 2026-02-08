/*
 * stm32_i2c_driver.h
 *
 *  Created on: Feb 8, 2026
 *      Author: Antigravity
 */

#ifndef BSP_DEVICE_DRIVER_PLATFORM_STM32_CONNECTIVITY_STM32_I2C_DRIVER_H_
#define BSP_DEVICE_DRIVER_PLATFORM_STM32_CONNECTIVITY_STM32_I2C_DRIVER_H_

#include "i2c_driver.h"
#include "stm32h7xx_hal.h"

// STM32 I2C 驱动结构体
typedef struct {
  i2c_driver_t base;       // 基类
  I2C_HandleTypeDef *hi2c; // STM32 HAL I2C 句柄
} stm32_i2c_driver_t;

// 构造函数
i2c_driver_t *stm32_i2c_driver_create(I2C_HandleTypeDef *hi2c);

#endif /* BSP_DEVICE_DRIVER_PLATFORM_STM32_CONNECTIVITY_STM32_I2C_DRIVER_H_ */
