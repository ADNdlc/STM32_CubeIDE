/*
 * qspi_hardware_adapter.h
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  STM32 QSPI 硬件适配器
 */

#ifndef PLATFORM_STM32_DEVICE_QSPI_HARDWARE_ADAPTER_H_
#define PLATFORM_STM32_DEVICE_QSPI_HARDWARE_ADAPTER_H_

#include "spi_adapter.h"
#include "stm32h7xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

// QSPI 硬件适配器配置
typedef struct {
  QSPI_HandleTypeDef *hqspi; // HAL QSPI 句柄
  uint32_t timeout_ms;       // 默认超时（ms）
  uint32_t flash_size;       // Flash 大小（2^n bytes）
  uint8_t use_4byte_addr;    // 使用4字节地址
} qspi_hw_adapter_config_t;

// 创建 QSPI 硬件适配器
spi_adapter_t *
qspi_hardware_adapter_create(const qspi_hw_adapter_config_t *config);

// 销毁 QSPI 硬件适配器
void qspi_hardware_adapter_destroy(spi_adapter_t *adapter);

// 获取 QSPI 适配器操作接口
const spi_adapter_ops_t *qspi_hardware_adapter_get_ops(void);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_STM32_DEVICE_QSPI_HARDWARE_ADAPTER_H_ */
