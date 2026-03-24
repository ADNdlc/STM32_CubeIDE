#ifndef DRIVERS_PLATFORM_STM32_SDMMC_STM32_SDMMC_ADAPTER_H_
#define DRIVERS_PLATFORM_STM32_SDMMC_STM32_SDMMC_ADAPTER_H_

#include "device/sdcard/sdcard_adapter.h"
#include "stm32h7xx_hal.h"

/**
 * @brief Create STM32 SDMMC adapter instance
 *
 * @param hsd Points to the HAL SD handle
 * @return sdcard_adapter_t* Pointer to adapter instance, or NULL on failure
 */
sdcard_adapter_t *stm32_sdmmc_adapter_create(SD_HandleTypeDef *hsd);

/**
 * @brief Destroy STM32 SDMMC adapter instance
 *
 * @param adapter Pointer to adapter instance
 */
void stm32_sdmmc_adapter_destroy(sdcard_adapter_t *adapter);

#endif /* DRIVERS_PLATFORM_STM32_SDMMC_STM32_SDMMC_ADAPTER_H_ */
