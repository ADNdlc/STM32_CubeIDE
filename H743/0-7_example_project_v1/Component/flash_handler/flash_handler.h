#ifndef COMPONENT_FLASH_HANDLER_FLASH_HANDLER_H_
#define COMPONENT_FLASH_HANDLER_FLASH_HANDLER_H_

#include "block_device.h"
#include "strategy/flash_strategy.h"

typedef enum {
  FLASH_EVENT_INSERTED,
  FLASH_EVENT_REMOVED,
  FLASH_EVENT_ERROR
} flash_event_t;

typedef enum {
  FLASH_STATUS_DISCONNECTED,
  FLASH_STATUS_READY,
  FLASH_STATUS_ERROR
} flash_status_t;

typedef void (*flash_event_cb_t)(const char *prefix, flash_event_t event);

// 初始化
int flash_handler_init(void);

// 注册带有路径前缀和策略的设备
int flash_handler_register(const char *prefix, block_device_t *dev,
                           flash_strategy_t *strategy);

// 状态轮询
void flash_handler_poll(void);

// 设置事件回调
void flash_handler_set_callback(flash_event_cb_t cb);

// High-level Operations
int flash_handler_read(const char *path, uint32_t offset, uint8_t *buf,
                       size_t size);
int flash_handler_write(const char *path, uint32_t offset, const uint8_t *buf,
                        size_t size);

#endif /* COMPONENT_FLASH_HANDLER_FLASH_HANDLER_H_ */
