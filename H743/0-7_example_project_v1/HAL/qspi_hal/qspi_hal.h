#ifndef HAL_QSPI_HAL_QSPI_HAL_H_
#define HAL_QSPI_HAL_QSPI_HAL_H_

#include "qspi_driver.h"
#include "sys.h"
#include <stddef.h>


#define QSPI_MEMSOURCE SYS_MEM_INTERNAL

typedef struct qspi_hal_t qspi_hal_t;

typedef struct {
  qspi_driver_ops_t base_vtable;
} qspi_hal_vtable_t;

struct qspi_hal_t {
  qspi_hal_vtable_t *vtable;
  qspi_driver_t *driver;
};

void qspi_hal_init(qspi_hal_t *self, qspi_driver_t *driver);
qspi_hal_t *qspi_hal_create(qspi_driver_t *driver);
void qspi_hal_destroy(qspi_hal_t *self);

/* Inline polymorphic calls */
static inline int qspi_hal_command(qspi_hal_t *self, qspi_command_t *cmd,
                                   uint32_t timeout) {
  if (self->driver && self->driver->ops->command) {
    return self->driver->ops->command(self->driver, cmd, timeout);
  }
  return -1;
}

static inline int qspi_hal_transmit(qspi_hal_t *self, const uint8_t *data,
                                    uint32_t timeout) {
  if (self->driver && self->driver->ops->transmit) {
    return self->driver->ops->transmit(self->driver, data, timeout);
  }
  return -1;
}

static inline int qspi_hal_receive(qspi_hal_t *self, uint8_t *data,
                                   uint32_t timeout) {
  if (self->driver && self->driver->ops->receive) {
    return self->driver->ops->receive(self->driver, data, timeout);
  }
  return -1;
}

static inline int qspi_hal_auto_polling(qspi_hal_t *self, qspi_command_t *cmd,
                                        qspi_command_t *cfg, uint32_t timeout) {
  if (self->driver && self->driver->ops->auto_polling) {
    return self->driver->ops->auto_polling(self->driver, cmd, cfg, timeout);
  }
  return -1;
}

static inline int qspi_hal_memory_mapped(qspi_hal_t *self,
                                         qspi_command_t *cmd) {
  if (self->driver && self->driver->ops->memory_mapped) {
    return self->driver->ops->memory_mapped(self->driver, cmd);
  }
  return -1;
}

#endif /* HAL_QSPI_HAL_QSPI_HAL_H_ */
