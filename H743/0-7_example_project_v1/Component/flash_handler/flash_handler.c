#include "flash_handler.h"
#include "elog.h"
#include "main.h"
#include "project_cfg.h"
#include "sys.h"
#include <string.h>

#define LOG_TAG "FlashHdlr"

#define MAX_MOUNT_POINTS 4 // 最大挂载点

typedef struct {
  char prefix[16];            // 前缀
  block_device_t *dev;        // 设备
  flash_strategy_t *strategy; // 策略
  flash_status_t status;      // 当前状态
  flash_device_type_t type;   // 设备类型
  uint32_t next_retry_tick;   // 下次尝试挂载的时间戳
} mount_point_t;

static mount_point_t mount_table[MAX_MOUNT_POINTS]; // 挂载表
static int mount_count = 0;
static flash_event_cb_t g_event_cb = NULL;

int flash_handler_init(void) {
  memset(mount_table, 0, sizeof(mount_table));
  mount_count = 0;
  g_event_cb = NULL;
  return 0;
}

void flash_handler_set_callback(flash_event_cb_t cb) { g_event_cb = cb; }

/**
 * @brief 注册带有路径前缀、策略和类型的设备
 */
int flash_handler_register(const char *prefix, block_device_t *dev,
                           flash_strategy_t *strategy,
                           flash_device_type_t type) {
  if (mount_count >= MAX_MOUNT_POINTS) {
    log_e("Too many mount points");
    return -1;
  }

  // 添加到挂载表 (初始状态设为 ABSENT)
  strncpy(mount_table[mount_count].prefix, prefix,
          sizeof(mount_table[mount_count].prefix) - 1);
  mount_table[mount_count].dev = dev;
  mount_table[mount_count].strategy = strategy;
  mount_table[mount_count].status = FLASH_STATUS_ABSENT;
  mount_table[mount_count].type = type;
  mount_table[mount_count].next_retry_tick = 0; // 首次轮询不延迟

  // 对于静态设备，直接尝试挂载
  if (type == FLASH_TYPE_STATIC) {
    if (BLOCK_DEV_INIT(dev) == 0) {
      if (strategy && strategy->ops->mount) {
        if (strategy->ops->mount(strategy, dev, prefix) == 0) {
          mount_table[mount_count].status = FLASH_STATUS_READY;
          log_i("Static mount point %s initialized as READY", prefix);
        } else {
          mount_table[mount_count].status = FLASH_STATUS_ERROR;
          log_e("Static mount point %s: Strategy mount failed", prefix);
        }
      }
    } else {
      mount_table[mount_count].status = FLASH_STATUS_ERROR;
      log_e("Static mount point %s: Block dev init failed", prefix);
    }
  }

  mount_count++;
  return 0;
}

/**
 * @brief 内部事件通知
 */
static void _notify_event(const char *prefix, flash_event_t event) {
  if (g_event_cb) {
    g_event_cb(prefix, event);
  }
}

/**
 * @brief 状态轮询
 */
void flash_handler_poll(void) {
  uint32_t now = HAL_GetTick();

  for (int i = 0; i < mount_count; i++) {
    mount_point_t *mp = &mount_table[i];
    if (!mp->dev)
      continue;

    // 1. 物理忙状态检查：如果设备正在忙（如DMA传输中），跳过轮询，避免干扰
    if (BLOCK_DEV_IS_BUSY(mp->dev)) {
      mp->status = FLASH_STATUS_BUSY;
      continue;
    } else {
      // 如果从繁忙转为空闲，且之前状态是BUSY，恢复为READY或ERROR
      if (mp->status == FLASH_STATUS_BUSY) {
        mp->status = FLASH_STATUS_READY; // 假设之前是READY
      }
    }

    // 2. 静态设备通常不需要轮询热插拔，但可以检查错误状态
    if (mp->type == FLASH_TYPE_STATIC) {
      if (mp->status == FLASH_STATUS_ERROR && now > mp->next_retry_tick) {
        log_d("Retrying static device %s init...", mp->prefix);
        if (BLOCK_DEV_INIT(mp->dev) == 0) {
          mp->status = FLASH_STATUS_READY;
          log_i("Static device %s recovered", mp->prefix);
        } else {
          mp->next_retry_tick = now + 5000;
        }
      }
      continue;
    }

    // 3. 轮询设备热插拔逻辑 (SD卡等)
    if (now < mp->next_retry_tick)
      continue;

    int present = BLOCK_DEV_IS_PRESENT(mp->dev);

    if (mp->status == FLASH_STATUS_READY) {
      // 已挂载，检测是否拔出
      if (present == 0) {
        log_w("Device %s removed, cleaning up...", mp->prefix);
        if (mp->strategy && mp->strategy->ops->unmount) {
          mp->strategy->ops->unmount(mp->strategy);
        }
        // 关键逻辑：拔出后必须反初始化，停止外设DMA/中断
        BLOCK_DEV_DEINIT(mp->dev);
        mp->status = FLASH_STATUS_ABSENT;
        mp->next_retry_tick = now + 1000;
        _notify_event(mp->prefix, FLASH_EVENT_REMOVED);
      }
    } else {
      // 未挂载 (ABSENT or ERROR)，检测是否插入
      if (present != 0) {
        log_i("Device %s detected, attempting mount...", mp->prefix);
        // 先初始化设备
        if (BLOCK_DEV_INIT(mp->dev) == 0) {
          // 再挂载文件系统
          if (mp->strategy && mp->strategy->ops->mount) {
            if (mp->strategy->ops->mount(mp->strategy, mp->dev, mp->prefix) ==
                0) {
              mp->status = FLASH_STATUS_READY;
              log_i("Device %s mounted successfully", mp->prefix);
              _notify_event(mp->prefix, FLASH_EVENT_INSERTED);
            } else {
              log_e("Strategy mount failed for %s", mp->prefix);
              mp->status = FLASH_STATUS_ERROR;
              mp->next_retry_tick = now + 2000;
            }
          }
        } else {
          // 初始化失败可能是临时通信问题或卡片损坏
          log_w("Block device init failed for %s", mp->prefix);
          mp->next_retry_tick = now + 2000;
        }
      } else {
        // 仍然不在，降频轮询
        mp->next_retry_tick = now + 500;
      }
    }
  }
}

/**
 * @brief 根据路径查找挂载点
 */
static mount_point_t *_find_mount_point(const char *path) {
  for (int i = 0; i < mount_count; i++) {
    if (strncmp(path, mount_table[i].prefix, strlen(mount_table[i].prefix)) ==
        0) {
      return &mount_table[i];
    }
  }
  if (!path)
    return NULL;
  size_t longest_match = 0;
  mount_point_t *best_match = NULL;

  for (int i = 0; i < mount_count; i++) {
    size_t len = strlen(mount_table[i].prefix);
    if (strncmp(path, mount_table[i].prefix, len) == 0) {
      if (len > longest_match) {
        longest_match = len;
        best_match = &mount_table[i];
      }
    }
  }
  return best_match;
}

static int _parse_mode(const char *mode) {
  int flags = 0;
  if (strchr(mode, 'r')) {
    if (strchr(mode, '+'))
      flags |= FLASH_O_RDWR;
    else
      flags |= FLASH_O_RDONLY;
  } else if (strchr(mode, 'w')) {
    flags |= FLASH_O_WRONLY | FLASH_O_CREAT | FLASH_O_TRUNC;
    if (strchr(mode, '+'))
      flags = (flags & ~FLASH_O_WRONLY) | FLASH_O_RDWR;
  } else if (strchr(mode, 'a')) {
    flags |= FLASH_O_WRONLY | FLASH_O_CREAT | FLASH_O_APPEND;
    if (strchr(mode, '+'))
      flags = (flags & ~FLASH_O_WRONLY) | FLASH_O_RDWR;
  }
  return flags;
}

// --- High-level File Operations ---

flash_file_t *flash_fopen(const char *path, const char *mode) {
  mount_point_t *mp = _find_mount_point(path);
  if (!mp || mp->status != FLASH_STATUS_READY)
    return NULL;

  void *handle = mp->strategy->ops->open(mp->strategy, path, _parse_mode(mode));
  if (!handle)
    return NULL;

  flash_file_t *file =
      (flash_file_t *)sys_malloc(SYS_MEM_INTERNAL, sizeof(flash_file_t));
  if (!file) {
    mp->strategy->ops->close(mp->strategy, handle);
    return NULL;
  }
  file->strategy = mp->strategy;
  file->handle = handle;
  return file;
}

int flash_fclose(flash_file_t *file) {
  if (!file)
    return -1;
  int res = file->strategy->ops->close(file->strategy, file->handle);
  sys_free(SYS_MEM_INTERNAL, file);
  return res;
}

size_t flash_fread(void *ptr, size_t size, size_t nmemb, flash_file_t *file) {
  if (!file)
    return 0;
  int res = file->strategy->ops->read(file->strategy, file->handle, ptr,
                                      size * nmemb);
  return (res >= 0) ? (size_t)res / size : 0;
}

size_t flash_fwrite(const void *ptr, size_t size, size_t nmemb,
                    flash_file_t *file) {
  if (!file)
    return 0;
  int res = file->strategy->ops->write(file->strategy, file->handle, ptr,
                                       size * nmemb);
  return (res >= 0) ? (size_t)res / size : 0;
}

int flash_fseek(flash_file_t *file, long offset, int whence) {
  if (!file)
    return -1;
  return file->strategy->ops->seek(file->strategy, file->handle,
                                   (int32_t)offset, whence);
}

long flash_ftell(flash_file_t *file) {
  if (!file)
    return -1;
  return (long)file->strategy->ops->tell(file->strategy, file->handle);
}

int flash_fsync(flash_file_t *file) {
  if (!file)
    return -1;
  return file->strategy->ops->sync(file->strategy, file->handle);
}

long flash_fsize(flash_file_t *file) {
  if (!file)
    return -1;
  return (long)file->strategy->ops->size(file->strategy, file->handle);
}

// Filesystem Operations

int flash_mkdir(const char *path) {
  mount_point_t *mp = _find_mount_point(path);
  if (!mp || mp->status != FLASH_STATUS_READY)
    return -1;
  return mp->strategy->ops->mkdir(mp->strategy, path);
}

int flash_unlink(const char *path) {
  mount_point_t *mp = _find_mount_point(path);
  if (!mp || mp->status != FLASH_STATUS_READY)
    return -1;
  return mp->strategy->ops->unlink(mp->strategy, path);
}

int flash_rename(const char *oldpath, const char *newpath) {
  mount_point_t *mp = _find_mount_point(oldpath);
  if (!mp || mp->status != FLASH_STATUS_READY)
    return -1;
  return mp->strategy->ops->rename(mp->strategy, oldpath, newpath);
}

int flash_stat(const char *path, flash_stat_t *st) {
  mount_point_t *mp = _find_mount_point(path);
  if (!mp || mp->status != FLASH_STATUS_READY)
    return -1;
  return mp->strategy->ops->stat(mp->strategy, path, st);
}

// Directory Operations

flash_dir_t *flash_opendir(const char *path) {
  mount_point_t *mp = _find_mount_point(path);
  if (!mp || mp->status != FLASH_STATUS_READY)
    return NULL;

  void *handle = mp->strategy->ops->opendir(mp->strategy, path);
  if (!handle)
    return NULL;

  flash_dir_t *dir =
      (flash_dir_t *)sys_malloc(SYS_MEM_INTERNAL, sizeof(flash_dir_t));
  if (!dir) {
    mp->strategy->ops->closedir(mp->strategy, handle);
    return NULL;
  }
  dir->strategy = mp->strategy;
  dir->handle = handle;
  return dir;
}

int flash_readdir(flash_dir_t *dir, flash_dirent_t *ent) {
  if (!dir)
    return -1;
  return dir->strategy->ops->readdir(dir->strategy, dir->handle, ent);
}

int flash_closedir(flash_dir_t *dir) {
  if (!dir)
    return -1;
  int res = dir->strategy->ops->closedir(dir->strategy, dir->handle);
  sys_free(SYS_MEM_INTERNAL, dir);
  return res;
}

// Compatibility / Oneshot

int flash_handler_read(const char *path, uint32_t offset, uint8_t *buf,
                       size_t size) {
  mount_point_t *mp = _find_mount_point(path);
  if (!mp || mp->status != FLASH_STATUS_READY)
    return -1;

  if (mp->strategy->ops->read_oneshot) {
    return mp->strategy->ops->read_oneshot(mp->strategy, path, offset, buf,
                                           size);
  }
  return -1;
}

int flash_handler_write(const char *path, uint32_t offset, const uint8_t *buf,
                        size_t size) {
  mount_point_t *mp = _find_mount_point(path);
  if (!mp || mp->status != FLASH_STATUS_READY)
    return -1;

  if (mp->strategy->ops->write_oneshot) {
    return mp->strategy->ops->write_oneshot(mp->strategy, path, offset, buf,
                                            size);
  }
  return -1;
}

// LVGL FS 端口获取文件系统实例
#include "lfs.h"
#include "strategy/lfs_strategy.h"
lfs_t *flash_handler_get_lfs(const char *prefix) {
  mount_point_t *mp = _find_mount_point(prefix);
  if (!mp || !mp->strategy || mp->status != FLASH_STATUS_READY)
    return NULL;
  return lfs_strategy_get_lfs(mp->strategy);
}
