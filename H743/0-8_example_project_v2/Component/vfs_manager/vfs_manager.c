#include "vfs_manager.h"
#include "Sys.h"
#include "elog.h"
#include <stdbool.h>
#include <string.h>

#define LOG_TAG "VFS"

static mount_point_t s_mount_points[VFS_MAX_MOUNT_POINTS];
static vfs_event_cb_t s_event_cb = NULL;

#define MONNITOR_TASK_CYCLE 2000
#define VFS_MAX_OPEN_FILES 8
#define VFS_MAX_OPEN_DIRS 4

typedef struct {
  mount_point_t *mp;
  vfs_file_t fs_file;
  bool in_use;
} vfs_fd_node_t;

typedef struct {
  mount_point_t *mp;
  vfs_dir_t fs_dir;
  bool in_use;
} vfs_dir_node_t;

static vfs_fd_node_t s_fds[VFS_MAX_OPEN_FILES];
static vfs_dir_node_t s_dirs[VFS_MAX_OPEN_DIRS];

/* ------------------ 内部辅助函数 ------------------ */

static mount_point_t *find_mount_point(const char *path,
                                       const char **rel_path) {
  const char *p = path;
  if (p[0] == '/')
    p++;

  for (int i = 0; i < VFS_MAX_MOUNT_POINTS; i++) {
    mount_point_t *mp = &s_mount_points[i];
    if (mp->name[0] == '\0' || !mp->is_mounted)
      continue;

    size_t len = strlen(mp->name);
    if (strncmp(p, mp->name, len) == 0 && (p[len] == '/' || p[len] == '\0')) {
      if (p[len] == '/')
        *rel_path = p + len + 1;
      else
        *rel_path = "";
      return mp;
    }
  }
  return NULL;
}

static int alloc_fd(mount_point_t *mp, vfs_file_t fs_file) {
  for (int i = 0; i < VFS_MAX_OPEN_FILES; i++) {
    if (!s_fds[i].in_use) {
      s_fds[i].mp = mp;
      s_fds[i].fs_file = fs_file;
      s_fds[i].in_use = true;
      return i;
    }
  }
  return -1;
}

/**
 * @brief 【新增】强制清理文件系统句柄并卸载，但保留挂载点存活
 */
static void vfs_force_unmount_fs(mount_point_t *mp) {
  if (!mp || !mp->is_mounted)
    return;

  // 1. 强制关闭所有关联的文件和目录句柄，防止拔卡导致堆内存泄漏！
  for (int j = 0; j < VFS_MAX_OPEN_FILES; j++) {
    if (s_fds[j].in_use && s_fds[j].mp == mp) {
      mp->fs_strategy->ops->close(s_fds[j].fs_file);
      s_fds[j].in_use = false;
    }
  }
  for (int j = 0; j < VFS_MAX_OPEN_DIRS; j++) {
    if (s_dirs[j].in_use && s_dirs[j].mp == mp) {
      mp->fs_strategy->ops->closedir(s_dirs[j].fs_dir);
      s_dirs[j].in_use = false;
    }
  }

  // 2. 调用底层策略卸载
  VFS_UNMOUNT(mp->fs_strategy, mp);
  mp->is_mounted = false;
}

static void vfs_internal_dev_cb(storage_device_t *self, dev_event_t event,
                                void *user_data) {
  mount_point_t *mp = (mount_point_t *)user_data;
  if (!mp)
    return;

  log_i("VFS Dev Event: [%s] -> %d", mp->name, event);

  if (event == DEVICE_EVENT_REMOVED) {
    mp->dev_state = STORAGE_STATUS_OFFLINE;
    if (mp->is_mounted) {
      log_w("Sudden removal detected for [%s], force unmounting FS.", mp->name);
      // 【修复】：只卸载 FS，绝对不能调用 vfs_unmount 抹除节点！
      vfs_force_unmount_fs(mp);
    }
  } else if (event == DEVICE_EVENT_INSERTED) {
    mp->dev_state = STORAGE_STATUS_OK;
    log_i("Device [%s] ready for re-mount.", mp->name);
  }

  if (s_event_cb)
    s_event_cb(mp->name, event);
}

bool vfs_point_is_mounted(const char *path_prefix) {
  const char *prefix = path_prefix;
  if (prefix[0] == '/')
    prefix++;

  for (int i = 0; i < VFS_MAX_MOUNT_POINTS; i++) {
    if (strcmp(s_mount_points[i].name, prefix) == 0) {
      return s_mount_points[i].is_mounted;
    }
  }
  return false;
}

/* ------------------ VFS 管理核心接口 ------------------ */

int vfs_init(void) {
  memset(s_mount_points, 0, sizeof(s_mount_points));
  memset(s_fds, 0, sizeof(s_fds));
  memset(s_dirs, 0, sizeof(s_dirs));
  log_i("VFS Manager initialized.");
  return 0;
}

int vfs_mount(const char *path_prefix, storage_device_t *dev,
              fs_strategy_t *strategy) {
  if (!path_prefix || !dev || !strategy)
    return -1;

  const char *prefix = path_prefix;
  if (prefix[0] == '/')
    prefix++;

  int free_idx = -1;
  for (int i = 0; i < VFS_MAX_MOUNT_POINTS; i++) {
    if (s_mount_points[i].name[0] == '\0') {
      if (free_idx == -1)
        free_idx = i;
    } else if (strcmp(s_mount_points[i].name, prefix) == 0) {
      return -2;
    }
  }
  if (free_idx == -1)
    return -3;

  mount_point_t *mp = &s_mount_points[free_idx];
  strncpy((char *)mp->name, prefix, sizeof(mp->name) - 1);
  mp->device = dev;
  mp->fs_strategy = strategy;
  mp->is_mounted = false;

  mp->dev_state = STORAGE_CHECK_ALIVE(dev);
  if (mp->dev_state == STORAGE_STATUS_NOT_INIT) {
    if (STORAGE_INIT(dev) == 0)
      mp->dev_state = STORAGE_CHECK_ALIVE(dev);
  }

  STORAGE_SET_CB(dev, vfs_internal_dev_cb, mp);

  if (mp->dev_state == STORAGE_STATUS_OK) {
    int m_ret = VFS_MOUNT(strategy, mp);
    if (m_ret == VFS_OK) {
      mp->is_mounted = true;
      mp->mount_err_code = VFS_OK;
      log_i("Mounted[%s] successfully.", mp->name);
    } else {
      mp->is_mounted = false;
      mp->mount_err_code = m_ret;

      // 【破除死锁】：如果在初始化时挂载失败（且不是无文件系统），强制降级硬件状态
      if (m_ret != VFS_ERR_NO_FS) {
        STORAGE_DEINIT(mp->device);
        mp->dev_state = STORAGE_STATUS_NOT_INIT;
      }
    }
  }
  return mp->mount_err_code;
}

int vfs_format(const char *path_prefix, vfs_op_notify_cb notify) {
  const char *prefix = path_prefix;
  if (prefix[0] == '/')
    prefix++;

  for (int i = 0; i < VFS_MAX_MOUNT_POINTS; i++) {
    if (strcmp(s_mount_points[i].name, prefix) == 0) {
      mount_point_t *mp = &s_mount_points[i];

      if (!mp->device || mp->dev_state != STORAGE_STATUS_OK)
        return VFS_ERR_NO_DEV;

      if (notify)
        notify(mp->name, true);
      int res = VFS_FORMAT(mp->fs_strategy, mp);
      if (notify)
        notify(mp->name, false);
      if (res == VFS_OK) {
        mp->mount_err_code = VFS_OK;
        if (VFS_MOUNT(mp->fs_strategy, mp) == VFS_OK) {
          mp->is_mounted = true;
        }
      }
      return res;
    }
  }
  return VFS_ERR_NOENT;
}

int vfs_unmount(const char *path_prefix) {
  const char *prefix = path_prefix;
  if (prefix[0] == '/')
    prefix++;

  for (int i = 0; i < VFS_MAX_MOUNT_POINTS; i++) {
    if (strcmp(s_mount_points[i].name, prefix) == 0) {
      // 彻底解绑：先卸载 FS，再抹除节点配置
      vfs_force_unmount_fs(&s_mount_points[i]);
      memset(&s_mount_points[i], 0, sizeof(mount_point_t));
      return 0;
    }
  }
  return -1;
}

void vfs_storage_monitor_task(void) {
  uint32_t now = sys_get_systick_ms();

  for (int i = 0; i < VFS_MAX_MOUNT_POINTS; i++) {
    mount_point_t *mp = &s_mount_points[i];
    if (mp->name[0] == '\0' || !mp->device)
      continue;

    storage_status_t current = STORAGE_CHECK_ALIVE(mp->device);

    if (current != mp->dev_state) {
      if (current == STORAGE_STATUS_OFFLINE ||
          current == STORAGE_STATUS_ERROR) {
        log_w("VFS: Device [%s] removed or error!", mp->name);
        vfs_force_unmount_fs(mp);
        STORAGE_DEINIT(mp->device);
      } else if (current == STORAGE_STATUS_OK) {
        log_i("VFS: Device [%s] detected!", mp->name);
        mp->mount_err_code = VFS_OK;
      }
      mp->dev_state = current;
    }

    if (!mp->is_mounted) {
      if (now - mp->last_retry_ms >= MONNITOR_TASK_CYCLE) {
        mp->last_retry_ms = now;

        if (mp->dev_state == STORAGE_STATUS_OK) {
          int m_ret = VFS_MOUNT(mp->fs_strategy, mp);
          if (m_ret == VFS_OK) {
            mp->is_mounted = true;
            log_i("VFS: Auto-mount [%s] successful.", mp->name);
          } else {
            mp->mount_err_code = m_ret;
            // 【破除轮询死锁】
            if (m_ret == VFS_ERR_NO_FS) {
              log_w("VFS: [%s] lacks filesystem. Waiting for format.",
                    mp->name);
            } else {
              // 挂载遭遇读写等底层错误，硬件必然假死。强制 DeInit 降级！
              log_e("VFS: Mount [%s] failed (Err:%d). Suspect HW glitch, "
                    "forcing Re-Init.",
                    mp->name, m_ret);
              STORAGE_DEINIT(mp->device);
              mp->dev_state = STORAGE_STATUS_NOT_INIT;
            }
          }
        } else {
          // 尝试硬件恢复 (状态为 NOT_INIT 时)
          log_d("VFS: Probing [%s] for hardware init...", mp->name);
          if (STORAGE_INIT(mp->device) == 0) {
            log_i("VFS: Device [%s] HW initialized.", mp->name);
            mp->dev_state =
                STORAGE_STATUS_OK; // 提速：不用等下一轮，立刻转为 OK
          } else {
            mp->dev_state = STORAGE_CHECK_ALIVE(mp->device);
          }
        }
      }
    }
  }
}

void vfs_set_event_callback(vfs_event_cb_t cb) { s_event_cb = cb; }

/* ------------------ 标准 POSIX 路由实现 ------------------ */

int vfs_open(const char *path, int flags) {
  const char *rel_path = NULL;
  mount_point_t *mp = find_mount_point(path, &rel_path);
  if (!mp)
    return -1;

  vfs_file_t fs_file = NULL;
  int res = mp->fs_strategy->ops->open(mp, &fs_file, rel_path, flags);
  if (res == 0) {
    int fd = alloc_fd(mp, fs_file);
    if (fd < 0) {
      mp->fs_strategy->ops->close(fs_file);
      return -2;
    }
    return fd;
  }
  return -3;
}

int vfs_read(int fd, void *buf, size_t len) {
  if (fd < 0 || fd >= VFS_MAX_OPEN_FILES || !s_fds[fd].in_use)
    return -1;
  return s_fds[fd].mp->fs_strategy->ops->read(s_fds[fd].fs_file, buf, len);
}

int vfs_write(int fd, const void *buf, size_t len) {
  if (fd < 0 || fd >= VFS_MAX_OPEN_FILES || !s_fds[fd].in_use)
    return -1;
  return s_fds[fd].mp->fs_strategy->ops->write(s_fds[fd].fs_file, buf, len);
}

int vfs_close(int fd) {
  if (fd < 0 || fd >= VFS_MAX_OPEN_FILES || !s_fds[fd].in_use)
    return -1;
  int res = s_fds[fd].mp->fs_strategy->ops->close(s_fds[fd].fs_file);
  s_fds[fd].in_use = false;
  return res;
}

int vfs_opendir(const char *path, vfs_dir_t *dir) {
  const char *rel_path = NULL;
  mount_point_t *mp = find_mount_point(path, &rel_path);
  if (!mp)
    return VFS_ERR_NOENT;

  vfs_dir_t fs_dir = NULL;
  int res = mp->fs_strategy->ops->opendir(mp, &fs_dir, rel_path);
  if (res == VFS_OK) {
    for (int i = 0; i < VFS_MAX_OPEN_DIRS; i++) {
      if (!s_dirs[i].in_use) {
        s_dirs[i].mp = mp;
        s_dirs[i].fs_dir = fs_dir;
        s_dirs[i].in_use = true;
        *dir = (vfs_dir_t)(uintptr_t)i;
        return VFS_OK;
      }
    }
    mp->fs_strategy->ops->closedir(fs_dir);
    return VFS_ERR_NO_MEM; // 句柄用尽
  }
  return res;
}

int vfs_readdir(vfs_dir_t dir, vfs_dirent_t *dirent) {
  int id = (int)(uintptr_t)dir;
  if (id < 0 || id >= VFS_MAX_OPEN_DIRS || !s_dirs[id].in_use)
    return VFS_ERR_INVAL;
  return s_dirs[id].mp->fs_strategy->ops->readdir(s_dirs[id].fs_dir, dirent);
}

int vfs_closedir(vfs_dir_t dir) {
  int id = (int)(uintptr_t)dir;
  if (id < 0 || id >= VFS_MAX_OPEN_DIRS || !s_dirs[id].in_use)
    return VFS_ERR_INVAL;
  int res = s_dirs[id].mp->fs_strategy->ops->closedir(s_dirs[id].fs_dir);
  s_dirs[id].in_use = false;
  return res;
}

int vfs_mkdir(const char *path) {
  const char *rel_path = NULL;
  mount_point_t *mp = find_mount_point(path, &rel_path);
  if (!mp)
    return VFS_ERR_NOENT;
  return mp->fs_strategy->ops->mkdir(mp, rel_path);
}

int vfs_unlink(const char *path) {
  const char *rel_path = NULL;
  mount_point_t *mp = find_mount_point(path, &rel_path);
  if (!mp)
    return VFS_ERR_NOENT;
  return mp->fs_strategy->ops->unlink(mp, rel_path);
}

int vfs_rename(const char *old_path, const char *new_path) {
  const char *old_rel = NULL;
  const char *new_rel = NULL;
  mount_point_t *mp_old = find_mount_point(old_path, &old_rel);
  mount_point_t *mp_new = find_mount_point(new_path, &new_rel);

  // 不支持跨挂载点重命名
  if (!mp_old || !mp_new || mp_old != mp_new)
    return VFS_ERR_INVAL;

  return mp_old->fs_strategy->ops->rename(mp_old, old_rel, new_rel);
}

int vfs_stat(const char *path, vfs_stat_t *st) {
  const char *rel_path = NULL;
  mount_point_t *mp = find_mount_point(path, &rel_path);
  if (!mp)
    return VFS_ERR_NOENT;
  return mp->fs_strategy->ops->stat(mp, rel_path, st);
}
