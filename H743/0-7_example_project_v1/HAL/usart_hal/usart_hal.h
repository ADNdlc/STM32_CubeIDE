#ifndef HAL_USART_HAL_USART_HAL_H_
#define HAL_USART_HAL_USART_HAL_H_

#include "usart_driver.h"
#include <stddef.h>

typedef struct usart_hal_t usart_hal_t;

typedef struct {
  usart_driver_ops_t base_vtable;
} usart_hal_vtable_t;

struct usart_hal_t{
  usart_hal_vtable_t *vtable;//uart行为
  //依赖驱动
  usart_driver_t *driver;
};


void usart_hal_init(usart_hal_t *self, usart_driver_t *driver);
usart_hal_t* usart_hal_create(usart_driver_t *driver);
void usart_hal_destroy(usart_hal_t *self);


/* 内联多态调用函数 */
static inline int usart_hal_send(usart_hal_t *self, const uint8_t *data, size_t len, uint32_t timeout) {
  return self->vtable->base_vtable.transmit((usart_driver_t*)self, data, len, timeout);
}

static inline int usart_hal_recv(usart_hal_t *self, uint8_t *buf, size_t len, uint32_t timeout) {
  return self->vtable->base_vtable.receive((usart_driver_t*)self, buf, len, timeout);
}

static inline int usart_hal_transmit_asyn(usart_hal_t *self, const uint8_t *data, size_t len) {
  return self->vtable->base_vtable.transmit_asyn((usart_driver_t*)self, data, len);
}

static inline int usart_hal_receive_asyn(usart_hal_t *self, uint8_t *buf, size_t len) {
  return self->vtable->base_vtable.receive_asyn((usart_driver_t*)self, buf, len);
}

static inline int usart_hal_set_callback(usart_hal_t *self, usart_callback_t cb,
                           void *cb_context) {
  return self->vtable->base_vtable.set_callback((usart_driver_t*)self, cb, cb_context);
}


#endif /* HAL_USART_HAL_USART_HAL_H_ */