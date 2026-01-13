#include "fatfs_strategy.h"
#include "HAL/rtc_hal/rtc_hal.h"
#include "block_device.h"
#include "diskio.h"
#include "elog.h"
#include "ff.h"
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
  block_dev_info_t info;
  BLOCK_DEV_GET_INFO(g_fatfs_devices[pdrv], &info);
  uint32_t addr = sector * info.block_size;
  uint32_t size = count * info.block_size;

  if (BLOCK_DEV_READ(g_fatfs_devices[pdrv], addr, buff, size) == 0) {
    return RES_OK;
  }
  return RES_ERROR;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
  if (pdrv >= FF_VOLUMES || !g_fatfs_devices[pdrv]) {
    return RES_PARERR;
  }
  block_dev_info_t info;
  BLOCK_DEV_GET_INFO(g_fatfs_devices[pdrv], &info);
  uint32_t addr = sector * info.block_size;
  uint32_t size = count * info.block_size;

  if (BLOCK_DEV_PROGRAM(g_fatfs_devices[pdrv], addr, buff, size) == 0) {
    return RES_OK;
  }
  return RES_ERROR;
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

static int _fatfs_mount(flash_strategy_t *self, block_device_t *dev) {
  fatfs_strategy_impl_t *impl = (fatfs_strategy_impl_t *)self;
  self->dev = dev;

  if (impl->pdrv >= FF_VOLUMES) {
    log_e("Invalid pdrv: %d", impl->pdrv);
    return -1;
  }

  g_fatfs_devices[impl->pdrv] = dev;

  char path[4] = "0:";
  path[0] = impl->pdrv + '0';

  FRESULT res = f_mount(&impl->fs, path, 1);
  if (res != FR_OK) {
    log_w("FatFS mount failed (%d), attempting to format...", res);
    BYTE work[FF_MAX_SS];
    MKFS_PARM opt = {FM_ANY, 0, 0, 0, 0};
    res = f_mkfs(path, &opt, work, sizeof(work));
    if (res != FR_OK) {
      log_e("FatFS format failed (%d)", res);
      return -1;
    }
    res = f_mount(&impl->fs, path, 1);
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
    char path[4] = "0:";
    path[0] = impl->pdrv + '0';
    f_mount(NULL, path, 0);
    g_fatfs_devices[impl->pdrv] = NULL;
    impl->mounted = false;
  }
  return 0;
}

static int _fatfs_read(flash_strategy_t *self, const char *path,
                       uint32_t offset, uint8_t *buf, size_t size) {
  fatfs_strategy_impl_t *impl = (fatfs_strategy_impl_t *)self;
  if (!impl->mounted)
    return -1;

  FIL file;
  FRESULT res = f_open(&file, path, FA_READ);
  if (res != FR_OK)
    return -1;

  if (offset > 0) {
    f_lseek(&file, offset);
  }

  UINT br;
  res = f_read(&file, buf, size, &br);
  f_close(&file);

  return (res == FR_OK) ? 0 : -1;
}

static int _fatfs_write(flash_strategy_t *self, const char *path,
                        uint32_t offset, const uint8_t *buf, size_t size) {
  fatfs_strategy_impl_t *impl = (fatfs_strategy_impl_t *)self;
  if (!impl->mounted)
    return -1;

  FIL file;
  BYTE mode = FA_WRITE | FA_OPEN_ALWAYS;
  FRESULT res = f_open(&file, path, mode);
  if (res != FR_OK)
    return -1;

  f_lseek(&file, offset);

  UINT bw;
  res = f_write(&file, buf, size, &bw);
  f_sync(&file);
  f_close(&file);

  return (res == FR_OK) ? 0 : -1;
}

static const flash_strategy_ops_t fatfs_ops = {
    .mount = _fatfs_mount,
    .unmount = _fatfs_unmount,
    .read = _fatfs_read,
    .write = _fatfs_write,
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
