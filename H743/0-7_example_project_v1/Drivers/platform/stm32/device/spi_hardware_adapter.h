/*
 * spi_hardware_adapter.h
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  STM32 标准 SPI 硬件适配器
 */

#ifndef PLATFORM_STM32_DEVICE_SPI_HARDWARE_ADAPTER_H_
#define PLATFORM_STM32_DEVICE_SPI_HARDWARE_ADAPTER_H_

#include "spi_adapter.h"
#include "stm32h7xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

// SPI 硬件适配器配置
typedef struct {
  SPI_HandleTypeDef *hspi; // HAL SPI 句柄
  GPIO_TypeDef *cs_port;   // CS 引脚端口
  uint16_t cs_pin;         // CS 引脚
  uint32_t timeout_ms;     // 默认超时（ms）
} spi_hw_adapter_config_t;

// 创建 SPI 硬件适配器
spi_adapter_t *
spi_hardware_adapter_create(const spi_hw_adapter_config_t *config);

// 销毁 SPI 硬件适配器
void spi_hardware_adapter_destroy(spi_adapter_t *adapter);

// 获取 SPI 适配器操作接口
const spi_adapter_ops_t *spi_hardware_adapter_get_ops(void);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_STM32_DEVICE_SPI_HARDWARE_ADAPTER_H_ */
