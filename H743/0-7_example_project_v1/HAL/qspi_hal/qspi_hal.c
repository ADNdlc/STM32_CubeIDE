#include "qspi_hal.h"
#include "sys.h"
#include <stdlib.h>

int _qspi_hal_command(qspi_driver_t *self, qspi_command_t *cmd,
                      uint32_t timeout) {
  qspi_hal_t *hal = (qspi_hal_t *)self;
  if (hal->driver && hal->driver->ops->command) {
    return hal->driver->ops->command(hal->driver, cmd, timeout);
  }
  return -1;
}

int _qspi_hal_transmit(qspi_driver_t *self, const uint8_t *data,
                       uint32_t timeout) {
  qspi_hal_t *hal = (qspi_hal_t *)self;
  if (hal->driver && hal->driver->ops->transmit) {
    return hal->driver->ops->transmit(hal->driver, data, timeout);
  }
  return -1;
}

int _qspi_hal_receive(qspi_driver_t *self, uint8_t *data, uint32_t timeout) {
  qspi_hal_t *hal = (qspi_hal_t *)self;
  if (hal->driver && hal->driver->ops->receive) {
    return hal->driver->ops->receive(hal->driver, data, timeout);
  }
  return -1;
}

int _qspi_hal_auto_polling(qspi_driver_t *self, qspi_command_t *cmd,
                           qspi_command_t *cfg, uint32_t timeout) {
  qspi_hal_t *hal = (qspi_hal_t *)self;
  if (hal->driver && hal->driver->ops->auto_polling) {
    return hal->driver->ops->auto_polling(hal->driver, cmd, cfg, timeout);
  }
  return -1;
}

int _qspi_hal_memory_mapped(qspi_driver_t *self, qspi_command_t *cmd) {
  qspi_hal_t *hal = (qspi_hal_t *)self;
  if (hal->driver && hal->driver->ops->memory_mapped) {
    return hal->driver->ops->memory_mapped(hal->driver, cmd);
  }
  return -1;
}

qspi_hal_vtable_t qspi_hal_vtable = {
    .base_vtable = {.command = _qspi_hal_command,
                    .transmit = _qspi_hal_transmit,
                    .receive = _qspi_hal_receive,
                    .auto_polling = _qspi_hal_auto_polling,
                    .memory_mapped = _qspi_hal_memory_mapped}};

void qspi_hal_init(qspi_hal_t *self, qspi_driver_t *driver) {
  self->vtable = &qspi_hal_vtable;
  self->driver = driver;
}

qspi_hal_t *qspi_hal_create(qspi_driver_t *driver) {
  qspi_hal_t *self =
      (qspi_hal_t *)sys_malloc(QSPI_MEMSOURCE, sizeof(qspi_hal_t));
  if (self) {
    qspi_hal_init(self, driver);
  }
  return self;
}

void qspi_hal_destroy(qspi_hal_t *self) {
  if (!self)
    return;
  sys_free(QSPI_MEMSOURCE, self);
}
