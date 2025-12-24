#ifndef DRIVERS_PLATFORM_STM32_SPI_STM32_SPI_H_
#define DRIVERS_PLATFORM_STM32_SPI_STM32_SPI_H_

#include "spi_driver.h"
#include "stm32h7xx_hal.h"
#include "sys.h"

// STM32 SPI Driver Struct
typedef struct {
  spi_driver_t parent;     // Base class
  SPI_HandleTypeDef *hspi; // HAL Handle
  GPIO_TypeDef *cs_port;   // Chip Select Port
  uint16_t cs_pin;         // Chip Select Pin
} stm32_spi_t;

/**
 * @brief Create a defined STM32 SPI driver instance
 * @param hspi Pointer to HAL SPI Handle
 * @param cs_port GPIO Port for CS
 * @param cs_pin GPIO Pin for CS
 * @return Pointer to base driver interface
 */
spi_driver_t *stm32_spi_create(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port,
                               uint16_t cs_pin);

/**
 * @brief Destroy the driver instance
 */
void stm32_spi_destroy(spi_driver_t *driver);

#endif /* DRIVERS_PLATFORM_STM32_SPI_STM32_SPI_H_ */
