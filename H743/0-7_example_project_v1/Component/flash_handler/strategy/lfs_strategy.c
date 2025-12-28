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
  flash_strategy_t parent;
  lfs_t lfs;
  struct lfs_config cfg;
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

static int _lfs_mount(flash_strategy_t *self, block_device_t *dev) {
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

  impl->cfg.read_size = 16;
  impl->cfg.prog_size = 16;
  impl->cfg.block_size = info.sector_size;
  impl->cfg.block_count = info.capacity / info.sector_size;
  impl->cfg.cache_size = 256;
  impl->cfg.lookahead_size = 32;
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

static int _lfs_read(flash_strategy_t *self, const char *path, uint32_t offset,
                     uint8_t *buf, size_t size) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  if (!impl->mounted)
    return -1;

  lfs_file_t file;
  const char *filename = path;
  if (path[0] == '/') {
    const char *second_slash = strchr(path + 1, '/');
    if (second_slash)
      filename = second_slash + 1; // skip prefix and leading slash
  }

  int err = lfs_file_open(&impl->lfs, &file, filename, LFS_O_RDONLY);
  if (err)
    return -1;

  if (offset > 0) {
    lfs_file_seek(&impl->lfs, &file, offset, LFS_SEEK_SET);
  }

  lfs_ssize_t res = lfs_file_read(&impl->lfs, &file, buf, size);
  lfs_file_close(&impl->lfs, &file);

  return (res >= 0) ? 0 : -1;
}

static int _lfs_write(flash_strategy_t *self, const char *path, uint32_t offset,
                      const uint8_t *buf, size_t size) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)self;
  if (!impl->mounted)
    return -1;

  lfs_file_t file;
  const char *filename = path;
  if (path[0] == '/') {
    const char *second_slash = strchr(path + 1, '/');
    if (second_slash)
      filename = second_slash + 1;
  }

  int err =
      lfs_file_open(&impl->lfs, &file, filename, LFS_O_WRONLY | LFS_O_CREAT);
  if (err)
    return -1;

  lfs_file_seek(&impl->lfs, &file, offset, LFS_SEEK_SET);
  lfs_ssize_t res = lfs_file_write(&impl->lfs, &file, buf, size);
  lfs_file_sync(&impl->lfs, &file);
  lfs_file_close(&impl->lfs, &file);

  return (res >= 0) ? 0 : -1;
}

static const flash_strategy_ops_t lfs_ops = {
    .mount = _lfs_mount,
    .unmount = _lfs_unmount,
    .read = _lfs_read,
    .write = _lfs_write,
};

flash_strategy_t *lfs_strategy_create(void) {
  lfs_strategy_impl_t *impl = (lfs_strategy_impl_t *)sys_malloc(
      LFS_STRATEGY_MEMSOURCE, sizeof(lfs_strategy_impl_t));
  if (impl) {
    memset(impl, 0, sizeof(lfs_strategy_impl_t));
    impl->parent.ops = &lfs_ops;
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
