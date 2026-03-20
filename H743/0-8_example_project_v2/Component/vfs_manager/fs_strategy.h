#ifndef COMPONENT_FLASH_HANDLER_STRATEGY_FLASH_STRATEGY_H_
#define COMPONENT_FLASH_HANDLER_STRATEGY_FLASH_STRATEGY_H_

#include "storage_interface.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

// 前置声明，避免循环包含
struct mount_point_t;
typedef struct mount_point_t mount_point_t;

// VFS 统一定义的操作标志 (POSIX-like)
#define VFS_O_RDONLY 0x0001
#define VFS_O_WRONLY 0x0002
#define VFS_O_RDWR (VFS_O_RDONLY | VFS_O_WRONLY)
#define VFS_O_CREAT 0x0100
#define VFS_O_EXCL 0x0200
#define VFS_O_TRUNC 0x0400
#define VFS_O_APPEND 0x0800

// VFS 返回值错误码
typedef enum {
  VFS_OK = 0,
  VFS_ERR_GENERAL = -1,
  VFS_ERR_NO_MEM = -2,
  VFS_ERR_NO_DEV = -3,
  VFS_ERR_NOENT = -4,
  VFS_ERR_EXISTS = -5,
  VFS_ERR_ISDIR = -6,
  VFS_ERR_NOTDIR = -7,
  VFS_ERR_NOTEMPTY = -8,
  VFS_ERR_BUSY = -9,
  VFS_ERR_INVAL = -10,
  VFS_ERR_NOSPC = -11,
  VFS_ERR_ROFS = -12,
  VFS_ERR_IO = -13,
  VFS_ERR_NO_FS = -14,
} vfs_err_t;

// 统一文件句柄
typedef void *vfs_file_t;
typedef void *vfs_dir_t;

// 文件状态结构
typedef struct vfs_stat_t {
  uint32_t size;
  bool is_dir;
} vfs_stat_t;

#define VFS_MAX_PATH_LEN 128

// 目录项结构
typedef struct vfs_dirent_t {
  char name[VFS_MAX_PATH_LEN];
  vfs_stat_t info;
} vfs_dirent_t;

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
};

#define VFS_MOUNT(strategy, mp) (strategy)->ops->mount(mp)
#define VFS_UNMOUNT(strategy, mp) (strategy)->ops->unmount(mp)
#define VFS_FORMAT(strategy, mp) (strategy)->ops->format(mp)

#endif /* COMPONENT_FLASH_HANDLER_STRATEGY_FLASH_STRATEGY_H_ */
