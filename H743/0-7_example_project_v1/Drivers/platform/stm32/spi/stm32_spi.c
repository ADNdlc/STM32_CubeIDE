#include "stm32_spi.h"
#include <stdlib.h>

#define STM32_SPI_MEMSOURCE SYS_MEM_INTERNAL

static int _stm32_spi_transmit(spi_driver_t *self, const uint8_t *data,
                               size_t size, uint32_t timeout) {
  stm32_spi_t *spi = (stm32_spi_t *)self;
  if (HAL_SPI_Transmit(spi->hspi, (uint8_t *)data, size, timeout) == HAL_OK) {
    return 0;
  }
  return -1;
}

static int _stm32_spi_receive(spi_driver_t *self, uint8_t *data, size_t size,
                              uint32_t timeout) {
  stm32_spi_t *spi = (stm32_spi_t *)self;
  if (HAL_SPI_Receive(spi->hspi, data, size, timeout) == HAL_OK) {
    return 0;
  }
  return -1;
}

static int _stm32_spi_transmit_receive(spi_driver_t *self,
                                       const uint8_t *tx_data, uint8_t *rx_data,
                                       size_t size, uint32_t timeout) {
  stm32_spi_t *spi = (stm32_spi_t *)self;
  if (HAL_SPI_TransmitReceive(spi->hspi, (uint8_t *)tx_data, rx_data, size,
                              timeout) == HAL_OK) {
    return 0;
  }
  return -1;
}

static void _stm32_spi_cs_control(spi_driver_t *self, uint8_t state) {
  stm32_spi_t *spi = (stm32_spi_t *)self;
  if (spi->cs_port) {
    HAL_GPIO_WritePin(spi->cs_port, spi->cs_pin,
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
  }
}

static const spi_driver_ops_t stm32_spi_ops = {
    .transmit = _stm32_spi_transmit,
    .receive = _stm32_spi_receive,
    .transmit_receive = _stm32_spi_transmit_receive,
    .cs_control = _stm32_spi_cs_control,
};

spi_driver_t *stm32_spi_create(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port,
                               uint16_t cs_pin) {
  stm32_spi_t *spi =
      (stm32_spi_t *)sys_malloc(STM32_SPI_MEMSOURCE, sizeof(stm32_spi_t));
  if (spi) {
    spi->parent.ops = &stm32_spi_ops;
    spi->parent.user_data = spi;
    spi->hspi = hspi;
    spi->cs_port = cs_port;
    spi->cs_pin = cs_pin;

    // Ensure CS is high initially (inactive)
    if (cs_port) {
      HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    }

    return &spi->parent;
  }
  return NULL;
}

void stm32_spi_destroy(spi_driver_t *driver) {
  if (driver) {
    sys_free(STM32_SPI_MEMSOURCE, driver);
  }
}
