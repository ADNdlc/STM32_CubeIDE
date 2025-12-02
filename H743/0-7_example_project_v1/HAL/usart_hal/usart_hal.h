/*
 * usart_hal.h
 *
 *  Created on: Dec 1, 2025
 *      Author: 12114
 */

#ifndef HAL_USART_HAL_USART_HAL_H_
#define HAL_USART_HAL_USART_HAL_H_

#include "usart_driver.h"
#include <stddef.h>

typedef struct {
  usart_driver_t *driver;
} usart_hal_t;

void usart_hal_init(usart_hal_t *self, usart_driver_t *driver);
int usart_hal_send(usart_hal_t *self, const uint8_t *data, size_t len);
int usart_hal_recv(usart_hal_t *self, uint8_t *buf, size_t len);

#endif /* HAL_USART_HAL_USART_HAL_H_ */
