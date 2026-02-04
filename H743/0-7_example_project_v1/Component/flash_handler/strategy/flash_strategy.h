#ifndef COMPONENT_FLASH_HANDLER_STRATEGY_FLASH_STRATEGY_H_
#define COMPONENT_FLASH_HANDLER_STRATEGY_FLASH_STRATEGY_H_

#include "block_device.h"
#include <stddef.h>
#include <stdint.h>

typedef struct flash_strategy_t flash_strategy_t;

#define FLASH_O_RDONLY 0x01
#define FLASH_O_WRONLY 0x02
#define FLASH_O_RDWR 0x03
#define FLASH_O_CREAT 0x04
#define FLASH_O_APPEND 0x08
#define FLASH_O_TRUNC 0x10

#define FLASH_SEEK_SET 0
#define FLASH_SEEK_CUR 1
#define FLASH_SEEK_END 2

typedef enum {
  FLASH_TYPE_REG = 0,
  FLASH_TYPE_DIR,
} flash_file_type_t;

typedef struct {
  size_t size;
  flash_file_type_t type;
} flash_stat_t;

typedef struct {
  char name[256];
  flash_file_type_t type;
  size_t size;
} flash_dirent_t;

typedef struct {
  // 基本接口
  int (*mount)(flash_strategy_t *self, block_device_t *dev,
               const char *mount_prefix);
  int (*unmount)(flash_strategy_t *self);

  // 文件操作
  void *(*open)(flash_strategy_t *self, const char *path, int flags);
  int (*close)(flash_strategy_t *self, void *file);
  int (*read)(flash_strategy_t *self, void *file, void *buf, size_t size);
  int (*write)(flash_strategy_t *self, void *file, const void *buf,
               size_t size);
  int (*seek)(flash_strategy_t *self, void *file, int32_t offset, int whence);
  int32_t (*tell)(flash_strategy_t *self, void *file);
  int (*sync)(flash_strategy_t *self, void *file);
  int32_t (*size)(flash_strategy_t *self, void *file);

  // 目录和文件系统操作
  int (*mkdir)(flash_strategy_t *self, const char *path);
  int (*unlink)(flash_strategy_t *self, const char *path);
  int (*rename)(flash_strategy_t *self, const char *old_path,
                const char *new_path);
  int (*stat)(flash_strategy_t *self, const char *path, flash_stat_t *st);

  // 目录流操作
  void *(*opendir)(flash_strategy_t *self, const char *path);
  int (*readdir)(flash_strategy_t *self, void *dir, flash_dirent_t *ent);
  int (*closedir)(flash_strategy_t *self, void *dir);

  // 旧的一键式读写接口 (可选，可通过 open/read/close 实现)
  int (*read_oneshot)(flash_strategy_t *self, const char *path, uint32_t offset,
                      uint8_t *buf, size_t size);
  int (*write_oneshot)(flash_strategy_t *self, const char *path,
                       uint32_t offset, const uint8_t *buf, size_t size);
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

#endif /* COMPONENT_FLASH_HANDLER_STRATEGY_FLASH_STRATEGY_H_ */
