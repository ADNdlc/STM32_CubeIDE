#ifndef DRIVERS_PLATFORM_STM32_SPI_STM32_SPI_H_
#define DRIVERS_PLATFORM_STM32_SPI_STM32_SPI_H_

#include "spi_driver.h"
#include "stm32h7xx_hal.h"
#include "sys.h"

// STM32 SPI Driver Struct
typedef struct
{
  spi_driver_t parent;     // Base class
  SPI_HandleTypeDef *hspi; // HAL Handle
  GPIO_TypeDef *cs_port;   // Chip Select Port
  uint16_t cs_pin;         // Chip Select Pin
} stm32_spi_t;

/**
 * @brief 创建stm32平台spi_driver驱动(需要关闭 Hardware NSS Signal, 由软件管理)
 * @param hspi 初始化好的 hspi
 * @param cs_port 软件片选端口
 * @param cs_pin  软件片选引脚
 * @return Pointer to base driver interface
 */
spi_driver_t *stm32_spi_create(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port,
                               uint16_t cs_pin);

/**
 * @brief Destroy the driver instance
 */
void stm32_spi_destroy(spi_driver_t *driver);

#endif /* DRIVERS_PLATFORM_STM32_SPI_STM32_SPI_H_ */
