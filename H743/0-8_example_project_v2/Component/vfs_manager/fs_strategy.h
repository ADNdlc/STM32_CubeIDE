#ifndef COMPONENT_FLASH_HANDLER_STRATEGY_FLASH_STRATEGY_H_
#define COMPONENT_FLASH_HANDLER_STRATEGY_FLASH_STRATEGY_H_

#include "storage_interface.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// 前置声明，避免循环包含
struct mount_point_t;
typedef struct mount_point_t mount_point_t;

// 统一文件句柄
typedef void* vfs_file_t;
typedef void* vfs_dir_t;

// 文件状态结构
typedef struct vfs_stat_t {
    uint32_t size;
    bool is_dir;
} vfs_stat_t;

typedef struct fs_strategy_t fs_strategy_t;

typedef struct {
    // 挂载与卸载
    int (*mount)(mount_point_t *mp);
    int (*unmount)(mount_point_t *mp);
    int (*format)(mount_point_t *mp);

    // 文件操作
    int (*open)(mount_point_t *mp, vfs_file_t *file, const char *path, int flags);
    int (*read)(vfs_file_t file, void *buf, size_t len);
    int (*write)(vfs_file_t file, const void *buf, size_t len);
    int (*lseek)(vfs_file_t file, off_t offset, int whence);
    int (*sync)(vfs_file_t file);
    int (*close)(vfs_file_t file);

    // 目录与节点操作
    int (*opendir)(mount_point_t *mp, vfs_dir_t *dir, const char *path);
    int (*readdir)(vfs_dir_t dir, void *dirent);
    int (*closedir)(vfs_dir_t dir);
    
    int (*mkdir)(mount_point_t *mp, const char *path);
    int (*unlink)(mount_point_t *mp, const char *path);
    int (*rename)(mount_point_t *mp, const char *old_path, const char *new_path);
    int (*stat)(mount_point_t *mp, const char *path, struct vfs_stat_t *st);

    // 特有功能扩展接口 (ioctl)
    int (*ioctl)(mount_point_t *mp, int cmd, void *arg);
} fs_ops_t;

/**
 * @brief 文件系统策略结构
 */
struct fs_strategy_t {
  const fs_ops_t *ops; // 策略对象
  const char *name;	   // 前缀
};

#define VFS_MOUNT(strategy, mp)          (strategy)->ops->mount(mp)
#define VFS_UNMOUNT(strategy, mp)        (strategy)->ops->unmount(mp)
#define VFS_FORMAT(strategy, mp)         (strategy)->ops->format(mp)

#endif /* COMPONENT_FLASH_HANDLER_STRATEGY_FLASH_STRATEGY_H_ */
