#include "spi_factory.h"
#include "factory_config.h"
#include "spi/stm32_spi.h"

static spi_driver_t *spi_drivers[SPI_MAX_DEVICES] = {NULL};

spi_driver_t *spi_driver_get(spi_device_id_t id) {
  if (id >= SPI_MAX_DEVICES)
    return NULL;

  if (spi_drivers[id] == NULL) {
    const spi_mapping_t *mapping = &spi_mappings[id];

#if (SPI_DRIVER_PLATFORM == PLATFORM_STM32)
    if (mapping->hspi) {
      spi_drivers[id] =
          stm32_spi_create(mapping->hspi, mapping->cs_port, mapping->cs_pin);
    }
#endif
  }

  return spi_drivers[id];
}
