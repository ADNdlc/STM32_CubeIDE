#ifndef DRIVERS_FACTORY_FLASH_FACTORY_H_
#define DRIVERS_FACTORY_FLASH_FACTORY_H_

#include "block_device.h"
#include "device_mapping.h"

/**
 * @brief Get a Block Device for Flash based on ID
 * @param id Device Identifier (e.g. FLASH_EXT_QSPI)
 * @return Block Device pointer or NULL
 */
block_device_t *flash_factory_get(flash_device_id_t id);

#endif /* DRIVERS_FACTORY_FLASH_FACTORY_H_ */
