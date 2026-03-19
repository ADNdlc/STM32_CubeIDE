#ifndef DRIVERS_PLATFORM_STM32_CONNECTIVITY_SDMMC_SDMMC_STORAGE_DRV_H_
#define DRIVERS_PLATFORM_STM32_CONNECTIVITY_SDMMC_SDMMC_STORAGE_DRV_H_

#include "storage_interface.h"

typedef struct {
  storage_device_t base;
  SD_HandleTypeDef *hsd;
} sdmmc_storage_device_t;

storage_device_t* sdmmc_storage_drv_get(const SD_HandleTypeDef* hsd);


#endif /* DRIVERS_PLATFORM_STM32_CONNECTIVITY_SDMMC_SDMMC_STORAGE_DRV_H_ */
