#ifndef HAL_SPI_HAL_SPI_HAL_H_
#define HAL_SPI_HAL_SPI_HAL_H_

#include "spi_driver.h"
#include "sys.h" // For memory macros if needed, or stdlib
#include <stddef.h>


// Memory allocation source
#define SPI_MEMSOURCE SYS_MEM_INTERNAL

typedef struct spi_hal_t spi_hal_t;

typedef struct {
  spi_driver_ops_t base_vtable;
} spi_hal_vtable_t;

struct spi_hal_t {
  spi_hal_vtable_t *vtable;
  spi_driver_t *driver;
};

void spi_hal_init(spi_hal_t *self, spi_driver_t *driver);
spi_hal_t *spi_hal_create(spi_driver_t *driver);
void spi_hal_destroy(spi_hal_t *self);

/* Inline polymorphic calls */
static inline int spi_hal_transmit(spi_hal_t *self, const uint8_t *data,
                                   size_t size, uint32_t timeout) {
  if (self->driver && self->driver->ops->transmit) {
    return self->driver->ops->transmit(self->driver, data, size, timeout);
  }
  return -1;
}

static inline int spi_hal_receive(spi_hal_t *self, uint8_t *data, size_t size,
                                  uint32_t timeout) {
  if (self->driver && self->driver->ops->receive) {
    return self->driver->ops->receive(self->driver, data, size, timeout);
  }
  return -1;
}

static inline int spi_hal_transmit_receive(spi_hal_t *self,
                                           const uint8_t *tx_data,
                                           uint8_t *rx_data, size_t size,
                                           uint32_t timeout) {
  if (self->driver && self->driver->ops->transmit_receive) {
    return self->driver->ops->transmit_receive(self->driver, tx_data, rx_data,
                                               size, timeout);
  }
  return -1;
}

static inline void spi_hal_cs_control(spi_hal_t *self, uint8_t state) {
  if (self->driver && self->driver->ops->cs_control) {
    self->driver->ops->cs_control(self->driver, state);
  }
}

#endif /* HAL_SPI_HAL_SPI_HAL_H_ */
