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
  FLASH_STATUS_ABSENT,
  FLASH_STATUS_READY,
  FLASH_STATUS_BUSY,
  FLASH_STATUS_ERROR
} flash_status_t;

typedef enum {
  FLASH_TYPE_STATIC, // 静态设备(如板载Flash)
  FLASH_TYPE_POLLING // 需要轮询的设备(如SD卡)
} flash_device_type_t;

typedef void (*flash_event_cb_t)(const char *prefix, flash_event_t event);

// 初始化
int flash_handler_init(void);

// 注册带有路径前缀、策略和类型的设备
int flash_handler_register(const char *prefix, block_device_t *dev,
                           flash_strategy_t *strategy,
                           flash_device_type_t type);

// 状态轮询
void flash_handler_poll(void);

// 设置事件回调
void flash_handler_set_callback(flash_event_cb_t cb);

// 结构定义
typedef struct {
  flash_strategy_t *strategy;
  void *handle;
} flash_file_t;

typedef struct {
  flash_strategy_t *strategy;
  void *handle;
} flash_dir_t;

// High-level File Operations
flash_file_t *flash_fopen(const char *path, const char *mode);
int flash_fclose(flash_file_t *file);
size_t flash_fread(void *ptr, size_t size, size_t nmemb, flash_file_t *file);
size_t flash_fwrite(const void *ptr, size_t size, size_t nmemb,
                    flash_file_t *file);
int flash_fseek(flash_file_t *file, long offset, int whence);
long flash_ftell(flash_file_t *file);
int flash_fsync(flash_file_t *file);
long flash_fsize(flash_file_t *file);

// Filesystem Operations
int flash_mkdir(const char *path);
int flash_unlink(const char *path);
int flash_rename(const char *oldpath, const char *newpath);
int flash_stat(const char *path, flash_stat_t *st);

// Directory Operations
flash_dir_t *flash_opendir(const char *path);
int flash_readdir(flash_dir_t *dir, flash_dirent_t *ent);
int flash_closedir(flash_dir_t *dir);

// 旧的一键式读写接口 (为了兼容)
int flash_handler_read(const char *path, uint32_t offset, uint8_t *buf,
                       size_t size);
int flash_handler_write(const char *path, uint32_t offset, const uint8_t *buf,
                        size_t size);

#endif /* COMPONENT_FLASH_HANDLER_FLASH_HANDLER_H_ */
