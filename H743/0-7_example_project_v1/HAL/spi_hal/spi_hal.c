#include "spi_hal.h"
#include "sys.h"
#include <stdlib.h>

// Forward declarations of internal wrapper functions if we needed a vtable for
// the HAL itself (like usart_hal did), but here we are mostly proxying.
// Ideally, the HAL object itself might override these if it had extra logic
// (like thread safety at HAL level). For now, we follow the pattern of
// wrapping.

int _spi_hal_transmit(spi_driver_t *self, const uint8_t *data, size_t size,
                      uint32_t timeout) {
  spi_hal_t *hal = (spi_hal_t *)self;
  if (hal->driver && hal->driver->ops->transmit) {
    return hal->driver->ops->transmit(hal->driver, data, size, timeout);
  }
  return -1;
}

int _spi_hal_receive(spi_driver_t *self, uint8_t *data, size_t size,
                     uint32_t timeout) {
  spi_hal_t *hal = (spi_hal_t *)self;
  if (hal->driver && hal->driver->ops->receive) {
    return hal->driver->ops->receive(hal->driver, data, size, timeout);
  }
  return -1;
}

int _spi_hal_transmit_receive(spi_driver_t *self, const uint8_t *tx_data,
                              uint8_t *rx_data, size_t size, uint32_t timeout) {
  spi_hal_t *hal = (spi_hal_t *)self;
  if (hal->driver && hal->driver->ops->transmit_receive) {
    return hal->driver->ops->transmit_receive(hal->driver, tx_data, rx_data,
                                              size, timeout);
  }
  return -1;
}

void _spi_hal_cs_control(spi_driver_t *self, uint8_t state) {
  spi_hal_t *hal = (spi_hal_t *)self;
  if (hal->driver && hal->driver->ops->cs_control) {
    hal->driver->ops->cs_control(hal->driver, state);
  }
}

spi_hal_vtable_t spi_hal_vtable = {
    .base_vtable = {.transmit = _spi_hal_transmit,
                    .receive = _spi_hal_receive,
                    .transmit_receive = _spi_hal_transmit_receive,
                    .cs_control = _spi_hal_cs_control}};

void spi_hal_init(spi_hal_t *self, spi_driver_t *driver) {
  self->vtable = &spi_hal_vtable;
  self->driver = driver;
}

spi_hal_t *spi_hal_create(spi_driver_t *driver) {
  spi_hal_t *self = (spi_hal_t *)sys_malloc(SPI_MEMSOURCE, sizeof(spi_hal_t));
  if (self) {
    spi_hal_init(self, driver);
  }
  return self;
}

void spi_hal_destroy(spi_hal_t *self) {
  if (!self)
    return;
  sys_free(SPI_MEMSOURCE, self);
}
