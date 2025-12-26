#include "flash_factory.h"
#include "device_mapping.h"
#include "qspi_factory.h"
#include "qspi_hal/qspi_hal.h"
#include "spi_factory.h"
#include "spi_hal/spi_hal.h"
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

      // 2. Wrap in HAL
      // Note: We create a new HAL instance here.
      // Ideally, the factory should manage singletons of HALs too if they share
      // state? But HAL is lightweight wrapper.
      spi_hal_t *hal = spi_hal_create(spi_drv);
      if (!hal)
        return NULL;

      // 3. Adapter
      w25q_adapter_t *adapter = w25q_spi_adapter_create(hal);
      if (!adapter) {
        spi_hal_destroy(hal);
        return NULL;
      }

      // 4. Device
      flash_devices[id] = w25qxx_create(adapter);
      if (!flash_devices[id]) {
        w25q_spi_adapter_destroy(adapter);
        spi_hal_destroy(hal);
      }

    } else if (mapping->type == FLASH_TYPE_QSPI) {
      qspi_driver_t *qspi_drv = qspi_driver_get(mapping->qspi_id);
      if (!qspi_drv)
        return NULL;

      qspi_hal_t *hal = qspi_hal_create(qspi_drv);
      if (!hal)
        return NULL;

      w25q_adapter_t *adapter = w25q_qspi_adapter_create(hal);
      if (!adapter) {
        qspi_hal_destroy(hal);
        return NULL;
      }

      flash_devices[id] = w25qxx_create(adapter);
      if (!flash_devices[id]) {
        w25q_qspi_adapter_destroy(adapter);
        qspi_hal_destroy(hal);
      }
    }
  }

  return flash_devices[id];
}
