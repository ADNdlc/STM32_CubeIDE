#ifndef COMPONENT_FLASH_HANDLER_FLASH_HANDLER_H_
#define COMPONENT_FLASH_HANDLER_FLASH_HANDLER_H_

#include "block_device.h"
#include "strategy/flash_strategy.h"


// 初始化
int flash_handler_init(void);

// 注册带有路径前缀和策略的设备
int flash_handler_register(const char *prefix, block_device_t *dev,
                           flash_strategy_t *strategy);

// High-level Operations
int flash_handler_read(const char *path, uint32_t offset, uint8_t *buf,
                       size_t size);
int flash_handler_write(const char *path, uint32_t offset, const uint8_t *buf,
                        size_t size);

#endif /* COMPONENT_FLASH_HANDLER_FLASH_HANDLER_H_ */
