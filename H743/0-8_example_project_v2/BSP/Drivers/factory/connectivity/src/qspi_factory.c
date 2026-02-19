/*
 * qspi_factory.c
 *
 *  Created on: Feb 16, 2026
 *      Author: Antigravity
 *
 *  QSPI 驱动工厂实现
 */

#include "qspi_factory.h"
#include <stddef.h>

#if (TARGET_PLATFORM == PLATFORM_STM32)
#include "qspi/stm32_qspi_driver.h"
#include "quadspi.h"

#endif

// 存储已创建的 QSPI 驱动实例指针数组
static qspi_driver_t *qspi_drivers[QSPI_MAX_DEVICES] = {NULL};

qspi_driver_t *qspi_driver_get(qspi_device_id_t id) {
  if (id >= QSPI_MAX_DEVICES) {
    return NULL;
  }

  if (qspi_drivers[id] == NULL) {
    const qspi_mapping_t *mapping = &qspi_mappings[id];
    if (mapping->resource == NULL) {
      return NULL;
    }

#if (TARGET_PLATFORM == PLATFORM_STM32)
    // 在 STM32 平台下，resource 被视为 stm32_qspi_config_t 指针
    qspi_drivers[id] =
        stm32_qspi_driver_create((QSPI_HandleTypeDef *)mapping->resource);
#else
#error "qspi_factory: 未定义 TARGET_PLATFORM 或平台不支持"
#endif
  }

  return qspi_drivers[id];
}
