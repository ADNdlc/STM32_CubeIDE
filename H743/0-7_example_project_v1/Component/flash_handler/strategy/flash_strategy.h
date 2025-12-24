#ifndef COMPONENT_FLASH_HANDLER_STRATEGY_FLASH_STRATEGY_H_
#define COMPONENT_FLASH_HANDLER_STRATEGY_FLASH_STRATEGY_H_

#include "block_device.h"
#include <stddef.h>
#include <stdint.h>


typedef struct flash_strategy_t flash_strategy_t;

typedef struct {
  int (*mount)(flash_strategy_t *self, block_device_t *dev);
  int (*unmount)(flash_strategy_t *self);
  int (*read)(flash_strategy_t *self, const char *path, uint32_t offset,
              uint8_t *buf, size_t size);
  int (*write)(flash_strategy_t *self, const char *path, uint32_t offset,
               const uint8_t *buf, size_t size);
  // Add open/close/etc if full FS is needed, but for now generic read/write
} flash_strategy_ops_t;

struct flash_strategy_t {
  const flash_strategy_ops_t *ops;
  block_device_t *dev;
};

#endif /* COMPONENT_FLASH_HANDLER_STRATEGY_FLASH_STRATEGY_H_ */
