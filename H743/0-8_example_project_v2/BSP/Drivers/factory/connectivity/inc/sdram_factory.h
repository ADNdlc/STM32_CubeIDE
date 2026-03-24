/*
 * sdram_factory.h
 *
 *  Created on: Dec 8, 2025
 *      Author: Antigravity
 */

#ifndef BSP_DEVICE_DRIVER_FACTORY_SDRAM_FACTORY_H_
#define BSP_DEVICE_DRIVER_FACTORY_SDRAM_FACTORY_H_

#include "dev_map.h"
#include "sdram_driver.h"

sdram_driver_t *sdram_driver_get(sdram_device_id_t id);

#endif /* BSP_DEVICE_DRIVER_FACTORY_SDRAM_FACTORY_H_ */
