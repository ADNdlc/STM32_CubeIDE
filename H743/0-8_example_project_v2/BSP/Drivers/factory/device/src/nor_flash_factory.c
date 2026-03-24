/*
 * nor_flash_factory.c
 *
 *  Created on: Feb 19, 2026
 *      Author: 12114
 */

#include "nor_flash_factory.h"
#include <stddef.h>
#include <stdint.h>

#if (DEV_NORFLASH == USE_W25Q)
#include "norflash/w25q_driver.h"
#include "qspi_factory.h"

#endif

static nor_flash_driver_t *nor_flash_drivers[NOR_FLASH_MAX] = {NULL};

nor_flash_driver_t *nor_flash_factory_creat(norflash_id_t id) {
  if (id >= NOR_FLASH_MAX) {
    return NULL;
  }

  if (nor_flash_drivers[id] == NULL) {
    const norflash_mapping_t *mapping = &norflash_mappings[id];
#if (DEV_NORFLASH == USE_W25Q)
    // 根据 mapping->resource (逻辑 ID) 获取 QSPI 驱动
    qspi_driver_t *qspi_drv =
        qspi_driver_get((qspi_device_id_t)(uintptr_t)mapping->resource);
    if (qspi_drv == NULL) {
      return NULL;
    }

    // 创建 W25Q 驱动
    nor_flash_drivers[id] = w25q_driver_create(qspi_drv, mapping->manual_info);
#endif
  }

  return nor_flash_drivers[id];
}
