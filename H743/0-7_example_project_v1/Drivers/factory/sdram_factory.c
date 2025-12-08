/*
 * sdram_factory.c
 *
 *  Created on: Dec 8, 2025
 *      Author: Antigravity
 */

#include "sdram_factory.h"
#include "device_mapping.h"
#include "stm32_sdram_driver.h"
#include <stddef.h>

static sdram_driver_t *sdram_drivers[SDRAM_MAX_DEVICES] = {NULL};

sdram_driver_t *sdram_driver_get(sdram_device_id_t id) {
  if (id >= SDRAM_MAX_DEVICES) {
    return NULL;
  }
  if (sdram_drivers[id] == NULL) {
    const sdram_mapping_t *mapping = &sdram_mappings[id];
    sdram_drivers[id] = stm32_sdram_create(mapping->hsdram);
  }
  return sdram_drivers[id];
}
