#ifndef DRIVERS_DEVICE_W25QXX_TRANSPORT_W25Q_QSPI_ADAPTER_H_
#define DRIVERS_DEVICE_W25QXX_TRANSPORT_W25Q_QSPI_ADAPTER_H_

#include "../w25q_adapter.h"
#include "qspi_driver.h"

w25q_adapter_t *w25q_qspi_adapter_create(qspi_driver_t *qspi_driver);
void w25q_qspi_adapter_destroy(w25q_adapter_t *adapter);

#endif /* DRIVERS_DEVICE_W25QXX_TRANSPORT_W25Q_QSPI_ADAPTER_H_ */