#ifndef DRIVERS_DEVICE_W25QXX_TRANSPORT_W25Q_SPI_ADAPTER_H_
#define DRIVERS_DEVICE_W25QXX_TRANSPORT_W25Q_SPI_ADAPTER_H_

#include "../w25q_adapter.h"
#include "spi_hal/spi_hal.h"

w25q_adapter_t *w25q_spi_adapter_create(spi_hal_t *hal);
void w25q_spi_adapter_destroy(w25q_adapter_t *adapter);

#endif /* DRIVERS_DEVICE_W25QXX_TRANSPORT_W25Q_SPI_ADAPTER_H_ */
