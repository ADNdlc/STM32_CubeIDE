#ifndef DRIVERS_FACTORY_FLASH_FACTORY_H_
#define DRIVERS_FACTORY_FLASH_FACTORY_H_

#include "block_device.h"
#include "device_mapping.h"

/**
 * @brief Create a Block Device for Flash based on TAG
 * @param tag Device Identifier (e.g. "FLASH_SPI_EXT")
 * @return Block Device pointer or NULL
 */
block_device_t *flash_factory_create(const char *tag);

#endif /* DRIVERS_FACTORY_FLASH_FACTORY_H_ */
