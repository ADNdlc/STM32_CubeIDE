/*
 * stm32_i2c_driver.h
 *
 *  Created on: Feb 8, 2026
 *      Author: Antigravity
 */

#ifndef BSP_DEVICE_DRIVER_PLATFORM_STM32_CONNECTIVITY_STM32_I2C_DRIVER_H_
#define BSP_DEVICE_DRIVER_PLATFORM_STM32_CONNECTIVITY_STM32_I2C_DRIVER_H_

#include "i2c_driver.h"
#include "i2c.h"
#include "stm32h7xx_hal.h"
#include <stdint.h>

// STM32 I2C 软件模拟配置
typedef struct{
  GPIO_TypeDef *scl_port;
  uint16_t scl_pin;
  GPIO_TypeDef *sda_port;
  uint16_t sda_pin;
  uint16_t delay_us; // 延时时间（微秒），控制 I2C 速度
} stm32_i2c_soft_config_t;

typedef struct{
  uint8_t is_soft;	// 0：硬件模式，1：软件模拟模式
  union{
    I2C_HandleTypeDef *hi2c;      // HAL 句柄
    stm32_i2c_soft_config_t *soft_config; // 软件配置
  } resource;
} stm32_i2c_config_t;

// STM32 I2C 驱动结构体
typedef struct{
  i2c_driver_t base; // 基类
  stm32_i2c_config_t config;
} stm32_i2c_driver_t;

// 修正后的构造函数宏
#define STM32_I2C_DRIVER_CREATE(stm32_i2c_config_t) \
  ((stm32_i2c_config_t)->is_soft ? \
  stm32_i2c_soft_driver_create(&(stm32_i2c_config_t)->resource.soft_config): \
  stm32_i2c_driver_create((stm32_i2c_config_t)->resource.hi2c))

i2c_driver_t *stm32_i2c_driver_create(I2C_HandleTypeDef *hi2c);
i2c_driver_t *stm32_i2c_soft_driver_create(const stm32_i2c_soft_config_t *config);

#endif /* BSP_DEVICE_DRIVER_PLATFORM_STM32_CONNECTIVITY_STM32_I2C_DRIVER_H_ */
