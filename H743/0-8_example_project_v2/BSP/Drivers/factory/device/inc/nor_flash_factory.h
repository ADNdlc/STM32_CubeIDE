/*
 * nor_flash_factory.h
 *
 *  Created on: Feb 19, 2026
 *      Author: 12114
 */

#ifndef DRIVERS_FACTORY_DEVICE_NOR_FLASH_FACTORY_H_
#define DRIVERS_FACTORY_DEVICE_NOR_FLASH_FACTORY_H_

#include "nor_flash_driver.h"
#include "dev_map.h"

nor_flash_driver_t *nor_flash_factory_creat(norflash_id_t id);

#endif /* DRIVERS_FACTORY_DEVICE_NOR_FLASH_FACTORY_H_ */
