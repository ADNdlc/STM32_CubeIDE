/*
 * stm32_spi_driver.c
 *
 *  Created on: Feb 16, 2026
 *      Author: Antigravity
 */

#include "stm32_spi_driver.h"
#include <stdlib.h>
//#include "MemPool.h"
#include <string.h>

#define SPI_MEMSOURCE SYS_MEM_INTERNAL

static int stm32_spi_transmit(spi_driver_t *self, const uint8_t *data,
                              uint16_t size, uint32_t timeout) {
  stm32_spi_driver_t *driver = (stm32_spi_driver_t *)self;
  HAL_StatusTypeDef status =
      HAL_SPI_Transmit(driver->hspi, (uint8_t *)data, size, timeout);
  return (status == HAL_OK) ? 0 : -1;
}

static int stm32_spi_receive(spi_driver_t *self, uint8_t *buffer, uint16_t size,
                             uint32_t timeout) {
  stm32_spi_driver_t *driver = (stm32_spi_driver_t *)self;
  HAL_StatusTypeDef status =
      HAL_SPI_Receive(driver->hspi, buffer, size, timeout);
  return (status == HAL_OK) ? 0 : -1;
}

static int stm32_spi_transmit_receive(spi_driver_t *self,
                                      const uint8_t *tx_data,
                                      uint8_t *rx_buffer, uint16_t size,
                                      uint32_t timeout) {
  stm32_spi_driver_t *driver = (stm32_spi_driver_t *)self;
  HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(
      driver->hspi, (uint8_t *)tx_data, rx_buffer, size, timeout);
  return (status == HAL_OK) ? 0 : -1;
}

static const spi_driver_ops_t stm32_spi_ops = {
    .transmit = stm32_spi_transmit,
    .receive = stm32_spi_receive,
    .transmit_receive = stm32_spi_transmit_receive,
};

spi_driver_t *stm32_spi_driver_create(SPI_HandleTypeDef *hspi) {
#ifdef USE_MEMPOOL
  stm32_spi_driver_t *driver = (stm32_spi_driver_t *)sys_malloc(
      SPI_MEMSOURCE, sizeof(stm32_spi_driver_t));
#else
  stm32_spi_driver_t *driver = (stm32_spi_driver_t *)malloc(sizeof(stm32_spi_driver_t));
#endif
  if (driver) {
    memset(driver, 0, sizeof(stm32_spi_driver_t));
    driver->base.ops = &stm32_spi_ops;
    driver->hspi = hspi;
  }
  return (spi_driver_t *)driver;
}
