/*
 * usart_hal.c
 *
 *  Created on: Dec 1, 2025
 *      Author: 12114
 */

#include "usart_hal.h"

void usart_hal_init(usart_hal_t *self, usart_driver_t *driver) {
  self->driver = driver;
}

int usart_hal_send(usart_hal_t *self, const uint8_t *data, size_t len) {
  if (self->driver && self->driver->ops->transmit) {
    return self->driver->ops->transmit(self->driver, data, len);
  }
  return -1;
}

int usart_hal_recv(usart_hal_t *self, uint8_t *buf, size_t len) {
  if (self->driver && self->driver->ops->receive) {
    return self->driver->ops->receive(self->driver, buf, len);
  }
  return -1;
}
