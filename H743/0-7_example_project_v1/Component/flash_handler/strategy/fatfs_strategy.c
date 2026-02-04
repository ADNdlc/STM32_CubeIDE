#include "fatfs_strategy.h"
#include "block_device.h"
#include "diskio.h"
#include "elog.h"
#include "ff.h"
#include "rtc_hal/rtc_hal.h"
#include "sys.h"
#include <stdbool.h>
#include <string.h>

#define LOG_TAG "FATFS_STRAT"
#define FATFS_STRATEGY_MEMSOURCE SYS_MEM_INTERNAL

typedef struct {
  flash_strategy_t parent;
  uint8_t pdrv;
  bool mounted;
  FATFS fs;
} fatfs_strategy_impl_t;

static block_device_t *g_fatfs_devices[FF_VOLUMES] = {0};

// --- FatFS diskio implementation ---

DSTATUS disk_status(BYTE pdrv) {
  if (pdrv >= FF_VOLUMES || !g_fatfs_devices[pdrv]) {
    return STA_NOINIT;
  }
  // Here we could check if dev is initialized, but for simplicity:
  return 0;
}

DSTATUS disk_initialize(BYTE pdrv) {
  if (pdrv >= FF_VOLUMES || !g_fatfs_devices[pdrv]) {
    return STA_NOINIT;
  }
  if (BLOCK_DEV_INIT(g_fatfs_devices[pdrv]) != 0) {
    return STA_NOINIT;
  }
  return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
  if (pdrv >= FF_VOLUMES || !g_fatfs_devices[pdrv]) {
    return RES_PARERR;
  }
  fatfs_strategy_impl_t *impl = NULL;
  // We need to find the impl that owns this pdrv to use its dma_buf
  // For simplicity, we assume one pdrv for now, but in a real system we'd
  // lookup.
  // Since diskio.c is global, we might need a way to find the strategy.
  // Actually, we can use a simpler approach: if not aligned, and we have
  // a global device, just use a local static aligned buffer (it's safe for
  // single task).
  static uint8_t g_dma_bounce_buf[512] __attribute__((aligned(32)));

  block_dev_info_t info;
  BLOCK_DEV_GET_INFO(g_fatfs_devices[pdrv], &info);

  for (UINT i = 0; i < count; i++) {
    uint32_t addr = (sector + i) * info.block_size;
    uint8_t *target = buff + (i * info.block_size);

    if (((uint32_t)target & 0x1F) == 0) {
      // Aligned, read direct
      if (BLOCK_DEV_READ(g_fatfs_devices[pdrv], addr, target,
                         info.block_size) != 0) {
        return RES_ERROR;
      }
    } else {
      // Unaligned, use bounce buffer
      if (BLOCK_DEV_READ(g_fatfs_devices[pdrv], addr, g_dma_bounce_buf,
                         info.block_size) != 0) {
        return RES_ERROR;
      }
      memcpy(target, g_dma_bounce_buf, info.block_size);
    }
  }

  return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
  if (pdrv >= FF_VOLUMES || !g_fatfs_devices[pdrv]) {
    return RES_PARERR;
  }
  static uint8_t g_dma_bounce_buf[512] __attribute__((aligned(32)));

  block_dev_info_t info;
  BLOCK_DEV_GET_INFO(g_fatfs_devices[pdrv], &info);

  for (UINT i = 0; i < count; i++) {
    uint32_t addr = (sector + i) * info.block_size;
    const uint8_t *source = buff + (i * info.block_size);

    if (((uint32_t)source & 0x1F) == 0) {
      // Aligned, write direct
      if (BLOCK_DEV_PROGRAM(g_fatfs_devices[pdrv], addr, source,
                            info.block_size) != 0) {
        return RES_ERROR;
      }
    } else {
      // Unaligned, use bounce buffer
      memcpy(g_dma_bounce_buf, source, info.block_size);
      if (BLOCK_DEV_PROGRAM(g_fatfs_devices[pdrv], addr, g_dma_bounce_buf,
                            info.block_size) != 0) {
        return RES_ERROR;
      }
    }
  }

  return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
  if (pdrv >= FF_VOLUMES || !g_fatfs_devices[pdrv]) {
    return RES_PARERR;
  }

  block_dev_info_t info;
  BLOCK_DEV_GET_INFO(g_fatfs_devices[pdrv], &info);

  switch (cmd) {
  case CTRL_SYNC:
    BLOCK_DEV_SYNC(g_fatfs_devices[pdrv]);
    return RES_OK;
  case GET_SECTOR_COUNT:
    *(LBA_t *)buff = info.capacity / info.block_size;
    return RES_OK;
  case GET_SECTOR_SIZE:
    *(WORD *)buff = info.block_size;
    return RES_OK;
  case GET_BLOCK_SIZE:
    *(DWORD *)buff = 1; // Not strictly used for FAT but can be sector size
    return RES_OK;
  default:
    return RES_PARERR;
  }
}

DWORD get_fattime(void) {
  rtc_date_t date;
  rtc_time_t time;

  if (rtc_hal_get_date(&date) == 0 && rtc_hal_get_time(&time) == 0) {
    return (DWORD)(date.year - 1980) << 25 | (DWORD)date.month << 21 |
           (DWORD)date.day << 16 | (DWORD)time.hour << 11 |
           (DWORD)time.minute << 5 | (DWORD)time.second >> 1;
  }
  return 0;
}

// --- Strategy Operations ---

static BYTE _map_flags(int flags) {
  BYTE mode = 0;
  if ((flags & FLASH_O_RDWR) == FLASH_O_RDWR)
    mode = FA_READ | FA_WRITE;
  else if (flags & FLASH_O_WRONLY)
    mode = FA_WRITE;
  else
    mode = FA_READ;

  if (flags & FLASH_O_CREAT) {
    if (flags & FLASH_O_TRUNC)
      mode |= FA_CREATE_ALWAYS;
    else
      mode |= FA_OPEN_ALWAYS;
  }

  if (flags & FLASH_O_APPEND) {
    mode |= FA_OPEN_APPEND;
  }

  return mode;
}

// --- Strategy Operations ---

static int _fatfs_mount(flash_strategy_t *self, block_device_t *dev,
                        const char *mount_prefix) {
  fatfs_strategy_impl_t *impl = (fatfs_strategy_impl_t *)self;
  self->dev = dev;

  if (!mount_prefix) {
    log_e("Mount prefix is NULL for FatFS");
    return -1;
  }

  g_fatfs_devices[impl->pdrv] = dev;

  FRESULT res = FR_NOT_READY;
  for (int retry = 0; retry < 3; retry++) {
    res = f_mount(&impl->fs, mount_prefix, 1);
    if (res == FR_OK)
      break;
    log_w("FatFS mount attempt %d failed (%d), retrying in 200ms...", retry + 1,
          res);
    sys_delay_ms(200);
  }

  if (res != FR_OK) {
    log_w("FatFS mount failed after retries (%d), attempting to format...",
          res);
    uint8_t *work = (uint8_t *)sys_malloc(FATFS_STRATEGY_MEMSOURCE, FF_MAX_SS);
    if (!work) {
      log_e("Failed to allocate format buffer");
      return -1;
    }
    MKFS_PARM opt = {FM_ANY, 0, 0, 0, 0};
    res = f_mkfs(mount_prefix, &opt, work, FF_MAX_SS);
    sys_free(FATFS_STRATEGY_MEMSOURCE, work);

    if (res != FR_OK) {
      log_e("FatFS format failed (%d)", res);
      return -1;
    }
    res = f_mount(&impl->fs, mount_prefix, 1);
    if (res != FR_OK) {
      log_e("FatFS mount after format failed (%d)", res);
      return -1;
    }
  }

  impl->mounted = true;
  log_i("FatFS mounted drive %d: successfully", impl->pdrv);
  return 0;
}

static int _fatfs_unmount(flash_strategy_t *self) {
  fatfs_strategy_impl_t *impl = (fatfs_strategy_impl_t *)self;
  if (impl->mounted) {
    char path[4] = {0};
    path[0] = impl->pdrv + '0';
    path[1] = ':';
    f_mount(NULL, path, 0);
    g_fatfs_devices[impl->pdrv] = NULL;
    impl->mounted = false;
  }
  return 0;
}

// File Operations

static void *_fatfs_open(flash_strategy_t *self, const char *path, int flags) {
  fatfs_strategy_impl_t *impl = (fatfs_strategy_impl_t *)self;
  if (!impl->mounted)
    return NULL;

  FIL *file = (FIL *)sys_malloc(FATFS_STRATEGY_MEMSOURCE, sizeof(FIL));
  if (!file)
    return NULL;

  FRESULT res = f_open(file, path, _map_flags(flags));
  if (res != FR_OK) {
    sys_free(FATFS_STRATEGY_MEMSOURCE, file);
    return NULL;
  }
  return (void *)file;
}

static int _fatfs_close(flash_strategy_t *self, void *file) {
  if (!file)
    return -1;
  FRESULT res = f_close((FIL *)file);
  sys_free(FATFS_STRATEGY_MEMSOURCE, file);
  return (res == FR_OK) ? 0 : -1;
}

static int _fatfs_read(flash_strategy_t *self, void *file, void *buf,
                       size_t size) {
  UINT br;
  FRESULT res = f_read((FIL *)file, buf, size, &br);
  return (res == FR_OK) ? (int)br : -1;
}

static int _fatfs_write(flash_strategy_t *self, void *file, const void *buf,
                        size_t size) {
  UINT bw;
  FRESULT res = f_write((FIL *)file, buf, size, &bw);
  return (res == FR_OK) ? (int)bw : -1;
}

static int _fatfs_seek(flash_strategy_t *self, void *file, int32_t offset,
                       int whence) {
  FIL *fp = (FIL *)file;
  FSIZE_t target = 0;
  switch (whence) {
  case FLASH_SEEK_SET:
    target = offset;
    break;
  case FLASH_SEEK_CUR:
    target = f_tell(fp) + offset;
    break;
  case FLASH_SEEK_END:
    target = f_size(fp) + offset;
    break;
  default:
    return -1;
  }
  FRESULT res = f_lseek(fp, target);
  return (res == FR_OK) ? 0 : -1;
}

static int32_t _fatfs_tell(flash_strategy_t *self, void *file) {
  return (int32_t)f_tell((FIL *)file);
}

static int _fatfs_sync(flash_strategy_t *self, void *file) {
  FRESULT res = f_sync((FIL *)file);
  return (res == FR_OK) ? 0 : -1;
}

static int32_t _fatfs_size(flash_strategy_t *self, void *file) {
  return (int32_t)f_size((FIL *)file);
}

// Filesystem Operations

static int _fatfs_mkdir(flash_strategy_t *self, const char *path) {
  FRESULT res = f_mkdir(path);
  return (res == FR_OK || res == FR_EXIST) ? 0 : -1;
}

static int _fatfs_unlink(flash_strategy_t *self, const char *path) {
  FRESULT res = f_unlink(path);
  return (res == FR_OK) ? 0 : -1;
}

static int _fatfs_rename(flash_strategy_t *self, const char *old_path,
                         const char *new_path) {
  FRESULT res = f_rename(old_path, new_path);
  return (res == FR_OK) ? 0 : -1;
}

static int _fatfs_stat(flash_strategy_t *self, const char *path,
                       flash_stat_t *st) {
  FILINFO fno;
  FRESULT res = f_stat(path, &fno);
  if (res != FR_OK)
    return -1;

  st->size = fno.fsize;
  st->type = (fno.fattrib & AM_DIR) ? FLASH_TYPE_DIR : FLASH_TYPE_REG;
  return 0;
}

// Directory Operations

static void *_fatfs_opendir(flash_strategy_t *self, const char *path) {
  DIR *dir = (DIR *)sys_malloc(FATFS_STRATEGY_MEMSOURCE, sizeof(DIR));
  if (!dir)
    return NULL;
  FRESULT res = f_opendir(dir, path);
  if (res != FR_OK) {
    sys_free(FATFS_STRATEGY_MEMSOURCE, dir);
    return NULL;
  }
  return (void *)dir;
}

static int _fatfs_readdir(flash_strategy_t *self, void *dir,
                          flash_dirent_t *ent) {
  FILINFO fno;
  FRESULT res = f_readdir((DIR *)dir, &fno);
  if (res != FR_OK || fno.fname[0] == 0)
    return -1;

  strncpy(ent->name, fno.fname, sizeof(ent->name) - 1);
  ent->type = (fno.fattrib & AM_DIR) ? FLASH_TYPE_DIR : FLASH_TYPE_REG;
  ent->size = fno.fsize;
  return 0;
}

static int _fatfs_closedir(flash_strategy_t *self, void *dir) {
  if (!dir)
    return -1;
  FRESULT res = f_closedir((DIR *)dir);
  sys_free(FATFS_STRATEGY_MEMSOURCE, dir);
  return (res == FR_OK) ? 0 : -1;
}

// Compatibility / Oneshot

static int _fatfs_read_oneshot(flash_strategy_t *self, const char *path,
                               uint32_t offset, uint8_t *buf, size_t size) {
  void *file = _fatfs_open(self, path, FLASH_O_RDONLY);
  if (!file)
    return -1;
  _fatfs_seek(self, file, offset, FLASH_SEEK_SET);
  int br = _fatfs_read(self, file, buf, size);
  _fatfs_close(self, file);
  return (br == (int)size) ? 0 : -1;
}

static int _fatfs_write_oneshot(flash_strategy_t *self, const char *path,
                                uint32_t offset, const uint8_t *buf,
                                size_t size) {
  void *file = _fatfs_open(self, path, FLASH_O_WRONLY | FLASH_O_CREAT);
  if (!file)
    return -1;
  _fatfs_seek(self, file, offset, FLASH_SEEK_SET);
  int bw = _fatfs_write(self, file, buf, size);
  _fatfs_sync(self, file);
  _fatfs_close(self, file);
  return (bw == (int)size) ? 0 : -1;
}

static const flash_strategy_ops_t fatfs_ops = {
    .mount = _fatfs_mount,
    .unmount = _fatfs_unmount,
    .open = _fatfs_open,
    .close = _fatfs_close,
    .read = _fatfs_read,
    .write = _fatfs_write,
    .seek = _fatfs_seek,
    .tell = _fatfs_tell,
    .sync = _fatfs_sync,
    .size = _fatfs_size,
    .mkdir = _fatfs_mkdir,
    .unlink = _fatfs_unlink,
    .rename = _fatfs_rename,
    .stat = _fatfs_stat,
    .opendir = _fatfs_opendir,
    .readdir = _fatfs_readdir,
    .closedir = _fatfs_closedir,
    .read_oneshot = _fatfs_read_oneshot,
    .write_oneshot = _fatfs_write_oneshot,
};

flash_strategy_t *fatfs_strategy_create(const fatfs_strategy_config_t *config) {
  fatfs_strategy_impl_t *impl = (fatfs_strategy_impl_t *)sys_malloc(
      FATFS_STRATEGY_MEMSOURCE, sizeof(fatfs_strategy_impl_t));
  if (impl) {
    memset(impl, 0, sizeof(fatfs_strategy_impl_t));
    impl->parent.ops = &fatfs_ops;
    if (config) {
      impl->pdrv = config->pdrv;
    }
    return &impl->parent;
  }
  return NULL;
}

void fatfs_strategy_destroy(flash_strategy_t *strategy) {
  if (strategy) {
    _fatfs_unmount(strategy);
    sys_free(FATFS_STRATEGY_MEMSOURCE, strategy);
  }
}
