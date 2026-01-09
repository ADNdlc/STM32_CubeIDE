#ifndef DRIVERS_FACTORY_SPI_FACTORY_H_
#define DRIVERS_FACTORY_SPI_FACTORY_H_

#include "device_mapping.h"
#include "spi_driver.h"


/**
 * @brief Get SPI Driver Instance
 * @param id SPI Device ID
 * @return Pointer to SPI Driver or NULL
 */
spi_driver_t *spi_driver_get(spi_device_id_t id);

#endif /* DRIVERS_FACTORY_SPI_FACTORY_H_ */
