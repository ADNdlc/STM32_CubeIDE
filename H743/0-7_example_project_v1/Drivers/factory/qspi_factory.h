#ifndef DRIVERS_FACTORY_QSPI_FACTORY_H_
#define DRIVERS_FACTORY_QSPI_FACTORY_H_

#include "device_mapping.h"
#include "qspi_driver.h"


qspi_driver_t *qspi_driver_get(qspi_device_id_t id);

#endif /* DRIVERS_FACTORY_QSPI_FACTORY_H_ */
