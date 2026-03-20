#ifndef COMPONENT_VFS_MANAGER_VFS_MANAGER_H_
#define COMPONENT_VFS_MANAGER_VFS_MANAGER_H_

#include "fs_strategy.h"
#include "storage_interface.h"

// 事件回调原型
typedef void (*vfs_event_cb_t)(const char *prefix, dev_event_t event);

#define VFS_MAX_MOUNT_POINTS 4

// TODO: 之后在 OS 环境下可替换为相应的 Mutex/Semaphore 类型，如 osMutexId_t
typedef void *vfs_lock_t;

#define VFS_LOCK(lock)   // osMutexWait((osMutexId_t)(lock), osWaitForever)
#define VFS_UNLOCK(lock) // osMutexRelease((osMutexId_t)(lock))

// 挂载点对象定义
typedef struct mount_point_t {
  const char name[16];              // 挂载前缀，如 "/data"
  storage_device_t *device;         // 绑定的物理存储介质
  storage_status_t dev_state;       // 底层设备状态
  const fs_strategy_t *fs_strategy; // 绑定的文件系统策略
  void *fs_context;                 // 文件系统私有上下文(如 lfs_t* 或 FATFS*)
  bool is_mounted;                  // 已挂载
  vfs_lock_t lock;                  // 单个挂载点的操作保护锁
  int mount_err_code;               // 记录挂载失败的底层错误码
} mount_point_t;

// 初始化
int vfs_init(void);

// 注册带有路径前缀、策略和类型的设备
int vfs_mount(const char *path_prefix, storage_device_t *dev,
              fs_strategy_t *strategy);
int vfs_unmount(const char *path_prefix);
int vfs_format(const char *path_prefix); // 格式化指定挂载点的文件系统

// 文件操作(POSIX 风格)
int vfs_open(const char *path, int flags);
int vfs_read(int fd, void *buf, size_t len);
int vfs_write(int fd, const void *buf, size_t len);
int vfs_close(int fd);

// 目录与节点操作
int vfs_opendir(const char *path, vfs_dir_t *dir);
int vfs_readdir(vfs_dir_t dir, vfs_dirent_t *dirent);
int vfs_closedir(vfs_dir_t dir);

int vfs_mkdir(const char *path);
int vfs_unlink(const char *path);
int vfs_rename(const char *old_path, const char *new_path);
int vfs_stat(const char *path, vfs_stat_t *st);

// 设置事件回调(所有挂载点共用)
void vfs_set_event_callback(vfs_event_cb_t cb);
// 挂载设备状态维护(空闲时低频轮询设备状态以提供移动设备的热插拔检查)
void vfs_storage_monitor_task(void);

#endif /* COMPONENT_VFS_MANAGER_VFS_MANAGER_H_ */