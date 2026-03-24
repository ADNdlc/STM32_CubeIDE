/*
 * stm32_i2c_soft_driver.h
 *
 *  Created on: Dec 13, 2025
 *      Author: Antigravity
 *
 *  STM32 软件模拟 I2C 驱动
 *  可指定任意 GPIO 引脚作为 SCL 和 SDA
 */

#ifndef PLATFORM_STM32_DEVICE_STM32_I2C_SOFT_DRIVER_H_
#define PLATFORM_STM32_DEVICE_STM32_I2C_SOFT_DRIVER_H_

#include "i2c_driver.h"
#include "stm32h7xx_hal.h"

// I2C 软件模拟配置
typedef struct {
  GPIO_TypeDef *scl_port; // SCL 引脚端口
  uint16_t scl_pin;       // SCL 引脚
  GPIO_TypeDef *sda_port; // SDA 引脚端口
  uint16_t sda_pin;       // SDA 引脚
  uint32_t delay_us;      // 延时时间（微秒），控制 I2C 速度
} stm32_i2c_soft_config_t;

// STM32 软件模拟 I2C 驱动结构体
typedef struct {
  i2c_driver_t base;              // 继承自 i2c_driver_t 基类
  stm32_i2c_soft_config_t config; // 配置
  uint8_t initialized;            // 初始化标志
} stm32_i2c_soft_driver_t;

/**
 * @brief 创建 STM32 软件模拟 I2C 驱动实例
 * @param config I2C 配置参数
 * @return 驱动实例指针，失败返回 NULL
 */
stm32_i2c_soft_driver_t *
stm32_i2c_soft_create(const stm32_i2c_soft_config_t *config);

/**
 * @brief 销毁 STM32 软件模拟 I2C 驱动实例
 * @param drv 驱动实例指针
 */
void stm32_i2c_soft_destroy(stm32_i2c_soft_driver_t *drv);

#endif /* PLATFORM_STM32_DEVICE_STM32_I2C_SOFT_DRIVER_H_ */
