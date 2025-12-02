/*
 * stm32_usart_driver.c
 *
 *  Created on: Dec 1, 2025
 *      Author: 12114
 */

#include "stm32_usart_driver.h"
#include <stdlib.h>

static int stm32_usart_transmit(usart_driver_t *self, const uint8_t *data,
                                size_t size) {
  stm32_usart_driver_t *driver = (stm32_usart_driver_t *)self;
  if (HAL_UART_Transmit(driver->huart, (uint8_t *)data, (uint16_t)size, 1000) ==
      HAL_OK) {
    return 0;
  }
  return -1;
}

static int stm32_usart_receive(usart_driver_t *self, uint8_t *buffer,
                               size_t size) {
  stm32_usart_driver_t *driver = (stm32_usart_driver_t *)self;
  if (HAL_UART_Receive(driver->huart, buffer, (uint16_t)size, 1000) == HAL_OK) {
    return 0;
  }
  return -1;
}

static const usart_driver_ops_t stm32_usart_ops = {
    .transmit = stm32_usart_transmit,
    .receive = stm32_usart_receive,
};

stm32_usart_driver_t *stm32_usart_driver_create(UART_HandleTypeDef *huart) {
  stm32_usart_driver_t *driver =
      (stm32_usart_driver_t *)malloc(sizeof(stm32_usart_driver_t));
  if (driver) {
    driver->base.ops = &stm32_usart_ops;
    driver->huart = huart;
  }
  return driver;
}
