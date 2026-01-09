#include "qspi_factory.h"
#include "factory_config.h"
#include "qspi/stm32_qspi.h"

static qspi_driver_t *qspi_drivers[QSPI_MAX_DEVICES] = {NULL};

qspi_driver_t *qspi_driver_get(qspi_device_id_t id) {
  if (id >= QSPI_MAX_DEVICES)
    return NULL;

  if (qspi_drivers[id] == NULL) {
    const qspi_mapping_t *mapping = &qspi_mappings[id];

#if (QSPI_DRIVER_PLATFORM == PLATFORM_STM32)
    if (mapping->hqspi) {
      qspi_drivers[id] = stm32_qspi_create(mapping->hqspi);
    }
#endif
  }

  return qspi_drivers[id];
}
