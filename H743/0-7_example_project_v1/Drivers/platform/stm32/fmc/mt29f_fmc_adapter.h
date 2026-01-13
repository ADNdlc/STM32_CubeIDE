#ifndef DRIVERS_DEVICE_MT29F4G08_TRANSPORT_MT29F_FMC_ADAPTER_H_
#define DRIVERS_DEVICE_MT29F4G08_TRANSPORT_MT29F_FMC_ADAPTER_H_

#include "../mt29f4g08_adapter.h"
#include "stm32h7xx_hal.h"

/**
 * @brief Create an FMC adapter instance
 *
 * @param hnand Pointer to initialized HAL NAND handle
 * @return mt29f_adapter_t*
 */
mt29f_adapter_t *mt29f_fmc_adapter_create(NAND_HandleTypeDef *hnand);

#endif /* DRIVERS_DEVICE_MT29F4G08_TRANSPORT_MT29F_FMC_ADAPTER_H_ */
