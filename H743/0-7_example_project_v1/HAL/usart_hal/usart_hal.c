#include "usart_hal.h"
#include <stdint.h>
#include <stdlib.h>


int _usart_hal_send(usart_driver_t *self, const uint8_t *data, size_t len,
                    uint32_t timeout) {
  // 修改类型转换
  usart_hal_t* hal = (usart_hal_t*)self;
  if (hal->driver && hal->driver->ops->transmit) {
    return USART_TRANSMIT(hal->driver, data, len, timeout);
  }
  return -1;
}

int _usart_hal_recv(usart_driver_t *self, uint8_t *buf, size_t len,
                    uint32_t timeout) {
  // 修改类型转换
  usart_hal_t* hal = (usart_hal_t*)self;
  if (hal->driver && hal->driver->ops->receive) {
    return USART_RECEIVE(hal->driver, buf, len, timeout);
  }
  return -1;
}

int _usart_hal_transmit_asyn(usart_driver_t *self, const uint8_t *data,
                             size_t len) {
  // 修改类型转换
  usart_hal_t* hal = (usart_hal_t*)self;
  if (hal->driver && hal->driver->ops->transmit_asyn) {
    return USART_TRANSMIT_ASYN(hal->driver, data, len);
  }
  return -1;
}

int _usart_hal_receive_asyn(usart_driver_t *self, uint8_t *buf, size_t len) {
  // 修改类型转换
  usart_hal_t* hal = (usart_hal_t*)self;
  if (hal->driver && hal->driver->ops->receive_asyn) {
    return USART_RECEIVE_ASYN(hal->driver, buf, len);
  }
  return -1;
}

int _usart_hal_set_callback(usart_driver_t *self, usart_callback_t cb,
                            void *cb_context) {
  // 修改类型转换
  usart_hal_t* hal = (usart_hal_t*)self;
  if (hal->driver && hal->driver->ops->set_callback) {
    return USART_SET_CALLBACK(hal->driver, cb, cb_context);
  }
  return -1;
}

// uart类虚表
usart_hal_vtable_t usart_hal_vtable = {
    .base_vtable = {
        .transmit = _usart_hal_send,
        .receive = _usart_hal_recv,
        .transmit_asyn = _usart_hal_transmit_asyn,
        .receive_asyn = _usart_hal_receive_asyn,
        .set_callback = _usart_hal_set_callback,
    }};

void usart_hal_init(usart_hal_t *self, usart_driver_t *driver) {
  self->vtable = &usart_hal_vtable;
  self->driver = driver;
}

usart_hal_t *usart_hal_create(usart_driver_t *driver) {
  usart_hal_t *self = (usart_hal_t *)malloc(sizeof(usart_hal_t));
  if (self) {
    usart_hal_init(self, driver);
  }
  return self;
}

void usart_hal_destroy(usart_hal_t *self) {
  if (!self) {
    return;
  }
  free(self);
}