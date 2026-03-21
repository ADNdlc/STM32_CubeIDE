#ifndef DRIVERS_PLATFORM_STM32_CONNECTIVITY_NOR_FLASH_NOR_FLASH_STORAGE_H_
#define DRIVERS_PLATFORM_STM32_CONNECTIVITY_NOR_FLASH_NOR_FLASH_STORAGE_H_

#include "storage_interface.h"
#include "nor_flash_driver.h"

storage_device_t *nor_flash_storage_create(nor_flash_driver_t *drv);

#endif /* DRIVERS_PLATFORM_STM32_CONNECTIVITY_NOR_FLASH_NOR_FLASH_STORAGE_H_ */
