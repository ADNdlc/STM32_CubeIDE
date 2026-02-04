#include "lfs_strategy.h"
#include "block_device.h"
#include "elog.h"
#include "lfs.h"
#include "sys.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "LFS_STRAT"
#define LFS_STRATEGY_MEMSOURCE SYS_MEM_INTERNAL

typedef struct {
  flash_strategy_t parent; // 策略接口
  lfs_t lfs;               // LittleFS 实例
  struct lfs_config cfg;   // LittleFS 配置
  lfs_strategy_config_t user_config;
  bool mounted;
} lfs_strategy_impl_t;

// --- LittleFS Block Device Callbacks ---

static int _lfs_bd_read(const struct lfs_config *c, lfs_block_t block,
                        lfs_off_t off, void *buffer, lfs_size_t size) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)c->context;
  uint32_t addr = block * c->block_size + off;
  return BLOCK_DEV_READ(impl->parent.dev, addr, buffer, size) == 0 ? LFS_ERR_OK
                                                                   : LFS_ERR_IO;
}

static int _lfs_bd_prog(const struct lfs_config *c, lfs_block_t block,
                        lfs_off_t off, const void *buffer, lfs_size_t size) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)c->context;
  uint32_t addr = block * c->block_size + off;
  return BLOCK_DEV_PROGRAM(impl->parent.dev, addr, buffer, size) == 0
             ? LFS_ERR_OK
             : LFS_ERR_IO;
}

static int _lfs_bd_erase(const struct lfs_config *c, lfs_block_t block) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)c->context;
  uint32_t addr = block * c->block_size;
  return BLOCK_DEV_ERASE(impl->parent.dev, addr, c->block_size) == 0
             ? LFS_ERR_OK
             : LFS_ERR_IO;
}

static int _lfs_bd_sync(const struct lfs_config *c) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)c->context;
  // BLOCK_DEV_SYNC usually calls wait_busy
  return BLOCK_DEV_SYNC(impl->parent.dev) == 0 ? LFS_ERR_OK : LFS_ERR_IO;
}

// --- Flash Strategy Operations Implementation ---

static int _map_flags(int flags) {
  int mode = 0;
  if ((flags & FLASH_O_RDWR) == FLASH_O_RDWR)
    mode = LFS_O_RDWR;
  else if (flags & FLASH_O_WRONLY)
    mode = LFS_O_WRONLY;
  else
    mode = LFS_O_RDONLY;

  if (flags & FLASH_O_CREAT)
    mode |= LFS_O_CREAT;
  if (flags & FLASH_O_APPEND)
    mode |= LFS_O_APPEND;
  if (flags & FLASH_O_TRUNC)
    mode |= LFS_O_TRUNC;

  return mode;
}

static const char *_normalize_path(const char *path) {
  const char *filename = path;
  if (path[0] == '/') {
    const char *second_slash = strchr(path + 1, '/');
    if (second_slash)
      filename = second_slash + 1; // skip prefix and leading slash
    else
      filename = path + 1; // skip leading slash if no prefix
  }
  return filename;
}

// --- Flash Strategy Operations Implementation ---

static int _lfs_mount(flash_strategy_t *self, block_device_t *dev,
                      const char *mount_prefix) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  self->dev = dev;

  // 1. Initialize block device
  if (BLOCK_DEV_INIT(dev) != 0) {
    log_e("Block device init failed");
    return -1;
  }

  // 2. Setup LittleFS config
  block_dev_info_t info;
  BLOCK_DEV_GET_INFO(dev, &info);

  memset(&impl->cfg, 0, sizeof(impl->cfg));
  impl->cfg.context = impl;
  impl->cfg.read = _lfs_bd_read;
  impl->cfg.prog = _lfs_bd_prog;
  impl->cfg.erase = _lfs_bd_erase;
  impl->cfg.sync = _lfs_bd_sync;

  if (impl->user_config.read_size > 0)
    impl->cfg.read_size = impl->user_config.read_size;
  else
    impl->cfg.read_size = (info.read_unit > 0) ? info.read_unit : 1;

  if (impl->user_config.prog_size > 0)
    impl->cfg.prog_size = impl->user_config.prog_size;
  else
    impl->cfg.prog_size = (info.prog_unit > 0) ? info.prog_unit : 1;

  impl->cfg.block_size = info.sector_size;
  impl->cfg.block_count = info.capacity / info.sector_size;

  if (impl->user_config.cache_size > 0)
    impl->cfg.cache_size = impl->user_config.cache_size;
  else
    impl->cfg.cache_size =
        (impl->cfg.prog_size > 256) ? impl->cfg.prog_size : 256;

  if (impl->user_config.lookahead_size > 0)
    impl->cfg.lookahead_size = impl->user_config.lookahead_size;
  else
    impl->cfg.lookahead_size = 32;

  if (impl->user_config.block_cycles > 0)
    impl->cfg.block_cycles = impl->user_config.block_cycles;
  else
    impl->cfg.block_cycles = 500;

  // 3. Mount
  int err = lfs_mount(&impl->lfs, &impl->cfg);
  if (err) {
    log_w("LFS mount failed (%d), formatting...", err);
    err = lfs_format(&impl->lfs, &impl->cfg);
    if (err) {
      log_e("LFS format failed (%d)", err);
      return -1;
    }
    err = lfs_mount(&impl->lfs, &impl->cfg);
    if (err) {
      log_e("LFS mount after format failed (%d)", err);
      return -1;
    }
  }

  impl->mounted = true;
  log_i("LFS mounted successfully");
  return 0;
}

static int _lfs_unmount(flash_strategy_t *self) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  if (impl->mounted) {
    lfs_unmount(&impl->lfs);
    impl->mounted = false;
  }
  if (self->dev) {
    BLOCK_DEV_DEINIT(self->dev);
    self->dev = NULL;
  }
  return 0;
}

// File Operations

static void *_lfs_open(flash_strategy_t *self, const char *path, int flags) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  if (!impl->mounted)
    return NULL;

  lfs_file_t *file =
      (lfs_file_t *)sys_malloc(LFS_STRATEGY_MEMSOURCE, sizeof(lfs_file_t));
  if (!file)
    return NULL;

  int err =
      lfs_file_open(&impl->lfs, file, _normalize_path(path), _map_flags(flags));
  if (err < 0) {
    sys_free(LFS_STRATEGY_MEMSOURCE, file);
    return NULL;
  }
  return (void *)file;
}

static int _lfs_close(flash_strategy_t *self, void *file) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  int res = lfs_file_close(&impl->lfs, (lfs_file_t *)file);
  sys_free(LFS_STRATEGY_MEMSOURCE, file);
  return (res >= 0) ? 0 : -1;
}

static int _lfs_read(flash_strategy_t *self, void *file, void *buf,
                     size_t size) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  lfs_ssize_t res = lfs_file_read(&impl->lfs, (lfs_file_t *)file, buf, size);
  return (int)res;
}

static int _lfs_write(flash_strategy_t *self, void *file, const void *buf,
                      size_t size) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  lfs_ssize_t res = lfs_file_write(&impl->lfs, (lfs_file_t *)file, buf, size);
  return (int)res;
}

static int _lfs_seek(flash_strategy_t *self, void *file, int32_t offset,
                     int whence) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  int lfs_whence = LFS_SEEK_SET;
  if (whence == FLASH_SEEK_CUR)
    lfs_whence = LFS_SEEK_CUR;
  else if (whence == FLASH_SEEK_END)
    lfs_whence = LFS_SEEK_END;

  lfs_soff_t res =
      lfs_file_seek(&impl->lfs, (lfs_file_t *)file, offset, lfs_whence);
  return (res >= 0) ? 0 : -1;
}

static int32_t _lfs_tell(flash_strategy_t *self, void *file) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  return (int32_t)lfs_file_tell(&impl->lfs, (lfs_file_t *)file);
}

static int _lfs_sync(flash_strategy_t *self, void *file) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  int res = lfs_file_sync(&impl->lfs, (lfs_file_t *)file);
  return (res >= 0) ? 0 : -1;
}

static int32_t _lfs_size(flash_strategy_t *self, void *file) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  return (int32_t)lfs_file_size(&impl->lfs, (lfs_file_t *)file);
}

// Filesystem Operations

static int _lfs_mkdir(flash_strategy_t *self, const char *path) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  int res = lfs_mkdir(&impl->lfs, _normalize_path(path));
  return (res >= 0 || res == LFS_ERR_EXIST) ? 0 : -1;
}

static int _lfs_unlink(flash_strategy_t *self, const char *path) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  int res = lfs_remove(&impl->lfs, _normalize_path(path));
  return (res >= 0) ? 0 : -1;
}

static int _lfs_rename(flash_strategy_t *self, const char *old_path,
                       const char *new_path) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  int res = lfs_rename(&impl->lfs, _normalize_path(old_path),
                       _normalize_path(new_path));
  return (res >= 0) ? 0 : -1;
}

static int _lfs_stat(flash_strategy_t *self, const char *path,
                     flash_stat_t *st) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  struct lfs_info info;
  int res = lfs_stat(&impl->lfs, _normalize_path(path), &info);
  if (res < 0)
    return -1;

  st->size = info.size;
  st->type = (info.type == LFS_TYPE_DIR) ? FLASH_TYPE_DIR : FLASH_TYPE_REG;
  return 0;
}

// Directory Operations

static void *_lfs_opendir(flash_strategy_t *self, const char *path) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  lfs_dir_t *dir =
      (lfs_dir_t *)sys_malloc(LFS_STRATEGY_MEMSOURCE, sizeof(lfs_dir_t));
  if (!dir)
    return NULL;

  int err = lfs_dir_open(&impl->lfs, dir, _normalize_path(path));
  if (err < 0) {
    sys_free(LFS_STRATEGY_MEMSOURCE, dir);
    return NULL;
  }
  return (void *)dir;
}

static int _lfs_readdir(flash_strategy_t *self, void *dir,
                        flash_dirent_t *ent) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  struct lfs_info info;
  int res = lfs_dir_read(&impl->lfs, (lfs_dir_t *)dir, &info);
  if (res <= 0)
    return -1;

  strncpy(ent->name, info.name, sizeof(ent->name) - 1);
  ent->type = (info.type == LFS_TYPE_DIR) ? FLASH_TYPE_DIR : FLASH_TYPE_REG;
  ent->size = info.size;
  return 0;
}

static int _lfs_closedir(flash_strategy_t *self, void *dir) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  int res = lfs_dir_close(&impl->lfs, (lfs_dir_t *)dir);
  sys_free(LFS_STRATEGY_MEMSOURCE, dir);
  return (res >= 0) ? 0 : -1;
}

// Oneshot

static int _lfs_read_oneshot(flash_strategy_t *self, const char *path,
                             uint32_t offset, uint8_t *buf, size_t size) {
  void *file = _lfs_open(self, path, FLASH_O_RDONLY);
  if (!file)
    return -1;
  _lfs_seek(self, file, offset, FLASH_SEEK_SET);
  int br = _lfs_read(self, file, buf, size);
  _lfs_close(self, file);
  return (br == (int)size) ? 0 : -1;
}

static int _lfs_write_oneshot(flash_strategy_t *self, const char *path,
                              uint32_t offset, const uint8_t *buf,
                              size_t size) {
  void *file = _lfs_open(self, path, FLASH_O_WRONLY | FLASH_O_CREAT);
  if (!file)
    return -1;
  _lfs_seek(self, file, offset, FLASH_SEEK_SET);
  int bw = _lfs_write(self, file, buf, size);
  _lfs_sync(self, file);
  _lfs_close(self, file);
  return (bw == (int)size) ? 0 : -1;
}

static const flash_strategy_ops_t lfs_ops = {
    .mount = _lfs_mount,
    .unmount = _lfs_unmount,
    .open = _lfs_open,
    .close = _lfs_close,
    .read = _lfs_read,
    .write = _lfs_write,
    .seek = _lfs_seek,
    .tell = _lfs_tell,
    .sync = _lfs_sync,
    .size = _lfs_size,
    .mkdir = _lfs_mkdir,
    .unlink = _lfs_unlink,
    .rename = _lfs_rename,
    .stat = _lfs_stat,
    .opendir = _lfs_opendir,
    .readdir = _lfs_readdir,
    .closedir = _lfs_closedir,
    .read_oneshot = _lfs_read_oneshot,
    .write_oneshot = _lfs_write_oneshot,
};

flash_strategy_t *lfs_strategy_create(const lfs_strategy_config_t *config) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)sys_malloc(
      LFS_STRATEGY_MEMSOURCE, sizeof(lfs_strategy_impl_t));
  if (impl) {
    memset(impl, 0, sizeof(lfs_strategy_impl_t));
    impl->parent.ops = &lfs_ops;
    if (config) {
      memcpy(&impl->user_config, config, sizeof(lfs_strategy_config_t));
    }
    return &impl->parent;
  }
  return NULL;
}

void lfs_strategy_destroy(flash_strategy_t *strategy) {
  if (strategy) {
    _lfs_unmount(strategy);
    sys_free(LFS_STRATEGY_MEMSOURCE, strategy);
  }
}

lfs_t *lfs_strategy_get_lfs(flash_strategy_t *strategy) {
  if (!strategy)
    return NULL;
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)strategy;
  return &impl->lfs;
}
