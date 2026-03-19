/*
 * storage_factory.c
 *
 *  Created on: Mar 19, 2026
 *      Author: Antigravity
 */

#include "sdmmc/sdmmc_storage_drv.h"
#include "storage_factory.h"
#include "dev_map_config.h"
#include <stddef.h>

#define USE_STORAGE_TYPE_SD_CARD
// #define USE_STORAGE_TYPE_NOR_FLASH

static storage_device_t *s_storage_devices[STORAGE_MAX_DEVICES] = {NULL};

storage_device_t *storage_factory_create(const storage_mapping_t *map) {
  // 调用类型对应的工厂
  if (map->type == STORAGE_TYPE_SD_CARD) {
    return sdmmc_storage_drv_get((SD_HandleTypeDef *)map->resource);
  }
#ifdef USE_STORAGE_TYPE_NOR_FLASH
  else if (map->type == STORAGE_TYPE_NOR_FLASH) {
    return norflash_storage_drv_get((norflash_mapping_t *)map->resource);
  }
#endif
  else {
    return NULL;
  }
}

storage_device_t *storage_factory_get(storage_device_id_t id) {
  if (id >= STORAGE_MAX_DEVICES) {
    return NULL;
  }
  if (s_storage_devices[id] == NULL) {
    const storage_mapping_t *mapping = &storage_mappings[id];
    s_storage_devices[id] = storage_factory_create(mapping);
  }
  return s_storage_devices[id];
}


