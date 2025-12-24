#ifndef DRIVERS_DEVICE_W25QXX_W25Q_ADAPTER_H_
#define DRIVERS_DEVICE_W25QXX_W25Q_ADAPTER_H_

#include "stm32h7xx_hal.h" // For errors or types if needed, or keep generic
#include <stddef.h>
#include <stdint.h>


typedef struct w25q_adapter_t w25q_adapter_t;

typedef struct {
  int (*init)(w25q_adapter_t *self);
  int (*read_id)(w25q_adapter_t *self, uint32_t *id);
  int (*read)(w25q_adapter_t *self, uint32_t addr, uint8_t *buf, size_t size);
  int (*write_enable)(w25q_adapter_t *self);
  int (*program_page)(w25q_adapter_t *self, uint32_t addr, const uint8_t *buf,
                      size_t size);
  int (*erase_sector)(w25q_adapter_t *self, uint32_t addr);
  int (*erase_block)(w25q_adapter_t *self, uint32_t addr);
  int (*erase_chip)(w25q_adapter_t *self);
  int (*wait_busy)(w25q_adapter_t *self, uint32_t timeout);
} w25q_adapter_ops_t;

struct w25q_adapter_t {
  const w25q_adapter_ops_t *ops;
  void *user_data;
};

#endif /* DRIVERS_DEVICE_W25QXX_W25Q_ADAPTER_H_ */
