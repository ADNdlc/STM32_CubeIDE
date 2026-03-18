#ifndef COMPONENT_FLASH_HANDLER_FLASH_HANDLER_H_
#define COMPONENT_FLASH_HANDLER_FLASH_HANDLER_H_

#include "storage_interface.h"
#include "fs_strategy.h"

typedef enum {
  DEVICE_EVENT_INSERTED,
  DEVICE_EVENT_REMOVED,
  FDEVICE_EVENT_ERROR
} dev_event_t;
// 事件回调原型
typedef void (*dev_event_cb_t)(const char *prefix, dev_event_t event);

#define VFS_MAX_MOUNT_POINTS 4

// 挂载点对象定义
typedef struct mount_point_t {
    const char name[16];             // 挂载前缀，如 "/data"
    storage_device_t *device;        // 绑定的物理存储介质
    const fs_strategy_t *fs_strategy;// 绑定的文件系统策略
    void *fs_context;                // 文件系统私有上下文(如 lfs_t* 或 FATFS*)
    bool is_mounted;
} mount_point_t;

// 初始化
int vfs_init(void);

// 注册带有路径前缀、策略和类型的设备
int vfs_mount(const char *path_prefix, storage_device_t *dev, fs_strategy_t *strategy);
int vfs_unmount(const char *path_prefix);

// 设置事件回调(所有挂载点共用)
void vfs_set_event_callback(dev_event_cb_t cb);

// 文件操作(POSIX 风格)
int vfs_open(const char *path, int flags);
int vfs_read(int fd, void *buf, size_t len);
int vfs_write(int fd, const void *buf, size_t len);
int vfs_close(int fd);
//...
#endif /* COMPONENT_FLASH_HANDLER_FLASH_HANDLER_H_ */
