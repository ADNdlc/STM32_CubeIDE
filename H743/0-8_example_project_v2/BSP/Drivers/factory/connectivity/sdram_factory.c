/*
 * sdram_factory.c
 *
 *  Created on: Dec 8, 2025
 *      Author: Antigravity
 */
#include "factory_config.h"
#include "sdram_factory.h"
#include "dev_map.h"

#if (SDRAM_DRIVER_PLATFORM == PLATFORM_STM32)
#include "fmc/stm32_sdram_driver.h"
#endif

static sdram_driver_t *sdram_drivers[SDRAM_MAX_DEVICES] = {NULL};

sdram_driver_t *sdram_driver_get(sdram_device_id_t id) {
  if (id >= SDRAM_MAX_DEVICES) {
    return NULL;
  }
  if (sdram_drivers[id] == NULL) {
    const sdram_mapping_t *mapping = &sdram_mappings[id];
    
    // 根据平台配置创建相应的SDRAM驱动实例
#if (SDRAM_DRIVER_PLATFORM == PLATFORM_STM32)
    sdram_drivers[id] = stm32_sdram_create((SDRAM_HandleTypeDef *)mapping->resource);
#else
#error "未定义SDRAM_DRIVER_PLATFORM或平台不支持"
#endif
  }
  return sdram_drivers[id];
}
