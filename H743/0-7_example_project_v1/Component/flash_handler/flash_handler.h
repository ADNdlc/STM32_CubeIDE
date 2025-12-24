#ifndef COMPONENT_FLASH_HANDLER_FLASH_HANDLER_H_
#define COMPONENT_FLASH_HANDLER_FLASH_HANDLER_H_

#include "block_device.h"
#include "strategy/flash_strategy.h"


// Initialize Handler
int flash_handler_init(void);

// Register a device with a path prefix and strategy
int flash_handler_register(const char *prefix, block_device_t *dev,
                           flash_strategy_t *strategy);

// High-level Operations
int flash_handler_read(const char *path, uint32_t offset, uint8_t *buf,
                       size_t size);
int flash_handler_write(const char *path, uint32_t offset, const uint8_t *buf,
                        size_t size);

#endif /* COMPONENT_FLASH_HANDLER_FLASH_HANDLER_H_ */
