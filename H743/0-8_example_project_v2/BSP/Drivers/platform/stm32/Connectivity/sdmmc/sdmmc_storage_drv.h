#ifndef DRIVERS_PLATFORM_STM32_CONNECTIVITY_SDMMC_SDMMC_STORAGE_DRV_H_
#define DRIVERS_PLATFORM_STM32_CONNECTIVITY_SDMMC_SDMMC_STORAGE_DRV_H_

#include "storage_interface.h"
#include "main.h"	// HAL库

typedef struct {
  storage_device_t base;
  SD_HandleTypeDef *hsd;
} sdmmc_storage_device_t;

storage_device_t* sdmmc_storage_drv_get(const SD_HandleTypeDef* hsd);
void sdmmc_storage_drv_on_interrupt(storage_device_t *self, bool is_inserted);


#endif /* DRIVERS_PLATFORM_STM32_CONNECTIVITY_SDMMC_SDMMC_STORAGE_DRV_H_ */
