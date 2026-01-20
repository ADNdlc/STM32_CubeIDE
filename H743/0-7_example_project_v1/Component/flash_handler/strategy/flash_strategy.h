#ifndef COMPONENT_FLASH_HANDLER_STRATEGY_FLASH_STRATEGY_H_
#define COMPONENT_FLASH_HANDLER_STRATEGY_FLASH_STRATEGY_H_

#include "block_device.h"
#include <stddef.h>
#include <stdint.h>

typedef struct flash_strategy_t flash_strategy_t;

typedef struct {
  // 基本接口
  int (*mount)(flash_strategy_t *self, block_device_t *dev,
               const char *mount_prefix); // 挂载设备加上前缀参数
  int (*unmount)(flash_strategy_t *self); // 卸载设备
  int (*read)(flash_strategy_t *self, const char *path, uint32_t offset,
              uint8_t *buf, size_t size);
  int (*write)(flash_strategy_t *self, const char *path, uint32_t offset,
               const uint8_t *buf, size_t size);
  // 完整文件操作
  // ...
} flash_strategy_ops_t;

/**
 * @brief Flash 策略
 *
 */
struct flash_strategy_t {
  const flash_strategy_ops_t *ops; // 策略对象
  block_device_t *dev;             // 设备对象
};

#define FLASH_STRATEGY_MOUNT(s, dev, prefix) (s)->ops->mount(s, dev, prefix)
#define FLASH_STRATEGY_UNMOUNT(s) (s)->ops->unmount(s)
#define FLASH_STRATEGY_READ(s, path, offset, buf, size)                        \
  (s)->ops->read(s, path, offset, buf, size)
#define FLASH_STRATEGY_WRITE(s, path, offset, buf, size)                       \
  (s)->ops->write(s, path, offset, buf, size)

#endif /* COMPONENT_FLASH_HANDLER_STRATEGY_FLASH_STRATEGY_H_ */
