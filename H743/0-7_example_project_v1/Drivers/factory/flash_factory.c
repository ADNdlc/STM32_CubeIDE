#include "flash_factory.h"
#include "device_mapping.h"
#include "qspi_factory.h"
#include "qspi_driver.h"
#include "spi_factory.h"
#include "spi_driver.h"
#include "w25qxx/w25qxx.h"
#include "w25qxx/transport/w25q_qspi_adapter.h"
#include "w25qxx/transport/w25q_spi_adapter.h"
#include <stddef.h>


// Static storage for devices using ID
static block_device_t *flash_devices[FLASH_MAX_DEVICES] = {NULL};

block_device_t *flash_factory_get(flash_device_id_t id) {
  if (id >= FLASH_MAX_DEVICES)
    return NULL;

  if (flash_devices[id] == NULL) {
    const flash_mapping_t *mapping = &flash_mappings[id];

    if (mapping->type == FLASH_TYPE_SPI) {
      // 1. Get SPI Driver from Factory (Dependency Injection)
      spi_driver_t *spi_drv = spi_driver_get(mapping->spi_id);
      if (!spi_drv)
        return NULL;

      // 2. Create adapter directly with SPI driver (no HAL needed)
      w25q_adapter_t *adapter = w25q_spi_adapter_create(spi_drv);
      if (!adapter) {
        return NULL;
      }

      // 3. Create device with adapter
      flash_devices[id] = w25qxx_create(adapter);
      if (!flash_devices[id]) {
        w25q_spi_adapter_destroy(adapter);
      }

    } else if (mapping->type == FLASH_TYPE_QSPI) {
      qspi_driver_t *qspi_drv = qspi_driver_get(mapping->qspi_id);
      if (!qspi_drv)
        return NULL;

      // Create adapter directly with QSPI driver (no HAL needed)
      w25q_adapter_t *adapter = w25q_qspi_adapter_create(qspi_drv);
      if (!adapter) {
        return NULL;
      }

      flash_devices[id] = w25qxx_create(adapter);
      if (!flash_devices[id]) {
        w25q_qspi_adapter_destroy(adapter);
      }
    }
  }

  return flash_devices[id];
}
