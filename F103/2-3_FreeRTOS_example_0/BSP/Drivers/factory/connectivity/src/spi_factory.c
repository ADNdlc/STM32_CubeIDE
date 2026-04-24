/*
 * spi_factory.c
 *
 *  Created on: Feb 16, 2026
 *      Author: Antigravity
 *
 *  SPI 驱动工厂实现
 */

#include "..\inc\spi_factory.h"
#include <stddef.h>

#if (TARGET_PLATFORM == PLATFORM_STM32)
#include "stm32_inc.h"
#endif

// 存储已创建的 SPI 驱动实例指针数组
static spi_driver_t *spi_drivers[SPI_MAX_DEVICES] = {NULL};

spi_driver_t *spi_driver_get(spi_device_id_t id) {
  if (id >= SPI_MAX_DEVICES) {
    return NULL;
  }

  if (spi_drivers[id] == NULL) {
    const spi_mapping_t *mapping = &spi_mappings[id];
    if (mapping->resource == NULL) {
      return NULL;
    }

#if (TARGET_PLATFORM == PLATFORM_STM32)
    // 在 STM32 平台下，直接传入 SPI_HandleTypeDef 指针
    spi_drivers[id] =
        STM32_SPI_DRIVER_CREATE((SPI_HandleTypeDef *)mapping->resource);
#else
#error "spi_factory: 未定义 TARGET_PLATFORM 或平台不支持"
#endif
  }

  return spi_drivers[id];
}
