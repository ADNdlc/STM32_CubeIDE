#ifndef DRIVERS_PLATFORM_STM32_QSPI_STM32_QSPI_H_
#define DRIVERS_PLATFORM_STM32_QSPI_STM32_QSPI_H_

#include "qspi_driver.h"
#include "stm32h7xx_hal.h"
#include "sys.h"

typedef struct {
  qspi_driver_t parent;
  QSPI_HandleTypeDef *hqspi;
} stm32_qspi_t;

/**
 * @brief Create STM32 QSPI driver
 */
qspi_driver_t *stm32_qspi_create(QSPI_HandleTypeDef *hqspi);
void stm32_qspi_destroy(qspi_driver_t *driver);

#endif /* DRIVERS_PLATFORM_STM32_QSPI_STM32_QSPI_H_ */
