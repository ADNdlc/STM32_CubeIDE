#include "fatfs_strategy.h"
#include "../vfs_manager.h"
#include "diskio.h"
#include "ff.h"
#include "rtc_factory.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MemPool.h"
#include "elog.h"

#define LOG_TAG "FATFS_STRAT"
#define FATFS_STRATEGY_MEMSOURCE SYS_MEM_INTERNAL

/*--------------------------------*/
/* diskio相关                 	  */
/*--------------------------------*/
static rtc_driver_t *rtc;
// 映射表：pdrv 对应 storage_device_t*
static storage_device_t *g_fatfs_devices[FF_VOLUMES] = {NULL};
uint8_t register_num = 0;

// 供 fatfs_strategy_mount 调用的注册函数
void fatfs_diskio_register(BYTE pdrv, storage_device_t *dev) {
  if (pdrv < FF_VOLUMES && dev != NULL) {
    g_fatfs_devices[pdrv] = dev;
    register_num++;
  }
}

DWORD get_fattime(void) {
  if (!rtc) {
    log_w(LOG_TAG, "RTC driver not found");
    return 0;
  }
  rtc_time_t time;
  rtc_date_t date;
  if (RTC_GET_TIME(rtc, &time) || RTC_GET_DATE(rtc, &date)) {
    log_w(LOG_TAG, "RTC driver ERR!");
    return 0;
  }
  return ((date.year - 1980) << 25) | ((date.month) << 21) |
         ((date.day) << 16) | ((time.hour) << 11) | ((time.minute) << 5) |
         ((time.second) >> 1);
}

DSTATUS disk_status(BYTE pdrv) {
  storage_device_t *dev = g_fatfs_devices[pdrv];
  if (!dev)
    return STA_NOINIT;

  storage_status_t status = STORAGE_CHECK_ALIVE(dev);
  if (status == STORAGE_STATUS_OFFLINE)
    return STA_NODISK;
  if (status == STORAGE_STATUS_NOT_INIT || status == STORAGE_STATUS_ERROR)
    return STA_NOINIT;

  return 0; // OK (0 代表正常)
}

DSTATUS disk_initialize(BYTE pdrv) {
  storage_device_t *dev = g_fatfs_devices[pdrv];
  if (!dev)
    return STA_NOINIT;
  rtc = rtc_driver_get(RTC_ID_INTERNAL);
  if (!rtc) {
    log_w(LOG_TAG, "RTC driver not found");
  }

  // 在此架构里，SD卡初始化由 Storage Manager 在挂载前完成了。
  // 所以这里只要检查状态即可
  return disk_status(pdrv);
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
  storage_device_t *dev = g_fatfs_devices[pdrv];
  if (!dev)
    return RES_NOTRDY;

  // 将 扇区数 转换为 字节地址 和 字节长度 (SD卡固定512)
  uint32_t addr = sector * dev->info.read_size;
  uint32_t len = count * dev->info.read_size;

  if (STORAGE_READ(dev, addr, buff, len) == 0) {
    return RES_OK;
  }
  return RES_ERROR;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
  storage_device_t *dev = g_fatfs_devices[pdrv];
  if (!dev)
    return RES_NOTRDY;

  uint32_t addr = sector * dev->info.write_size;
  uint32_t len = count * dev->info.write_size;

  if (STORAGE_WRITE(dev, addr, buff, len) == 0) {
    return RES_OK;
  }
  return RES_ERROR;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
  storage_device_t *dev = g_fatfs_devices[pdrv];
  if (!dev)
    return RES_NOTRDY;

  DRESULT res = RES_ERROR;
  switch (cmd) {
  case CTRL_SYNC:
    // 等待设备空闲，SD卡通过轮询内部状态判断，我们的 STORAGE_WRITE
    // 已经是阻塞同步的了
    res = RES_OK;
    break;
  case GET_SECTOR_COUNT: // 获取扇区总数
    *(LBA_t *)buff = dev->info.total_size / dev->info.erase_size;
    res = RES_OK;
    break;
  case GET_SECTOR_SIZE: // 获取扇区大小
    *(WORD *)buff = dev->info.erase_size;
    res = RES_OK;
    break;
  case GET_BLOCK_SIZE:  // 擦除块大小 (以扇区为单位)
    *(DWORD *)buff = 1; // 简单的写法，告诉 FatFs 每次擦 1 个扇区即可
    res = RES_OK;
    break;
  }
  return res;
}

/*--------------------------------*/
/* ops实现                 	  */
/*--------------------------------*/
static int fatfs_vfs_mount(mount_point_t *mp) {
  fatfs_strategy_t *strat = (fatfs_strategy_t *)mp->fs_strategy;

  // 1. 【核心桥接】将底层物理设备 device 注册到 diskio 的指定盘符上
  fatfs_diskio_register(strat->pdrv, mp->device);

  // 2. 调用 FatFs 真正的挂载
  // 参数 1：立即挂载，如果在这一步因为没插卡导致失败，我们要如实返回
  FRESULT res = f_mount(&strat->fs, strat->drive_num, 1);

  if (res == FR_OK) {
    mp->is_mounted = true;
    return 0; // 挂载成功
  } else {
    // 如果挂载失败，注销 diskio 映射以防空指针误用
    fatfs_diskio_register(strat->pdrv, NULL);
    return -1;
  }
}

static int fatfs_vfs_unmount(mount_point_t *mp) {
  fatfs_strategy_t *strat = (fatfs_strategy_t *)mp->fs_strategy;
  f_mount(NULL, strat->drive_num, 0);
  fatfs_diskio_register(strat->pdrv, NULL);
  mp->is_mounted = false;
  return VFS_OK;
}

static int fatfs_vfs_format(mount_point_t *mp) {
  fatfs_strategy_t *strat = (fatfs_strategy_t *)mp->fs_strategy;
  // TODO: 提供 mkfs 参数选项
  FRESULT res = f_mkfs(strat->drive_num, 0, 0, FF_MAX_SS);
  return (res == FR_OK) ? VFS_OK : -res;
}

static int fatfs_vfs_open(mount_point_t *mp, vfs_file_t *file, const char *path,
                          int flags) {
  fatfs_strategy_t *strat = (fatfs_strategy_t *)mp->fs_strategy;

  // 分配单独的 FIL 对象
#ifdef USE_MEMPOOL
  FIL *fp = sys_malloc(FATFS_STRATEGY_MEMSOURCE, sizeof(FIL));
#else
  FIL *fp = malloc(sizeof(FIL));
#endif
  if (!fp)
    return VFS_ERR_NO_MEM;

  char full_path[64];
  if (path[0] == '/')
    path++;
  snprintf(full_path, sizeof(full_path), "%s/%s", strat->drive_num, path);

  BYTE mode = 0;
  if ((flags & VFS_O_RDWR) == VFS_O_RDWR)
    mode |= (FA_READ | FA_WRITE);
  else if (flags & VFS_O_WRONLY)
    mode |= FA_WRITE;
  else
    mode |= FA_READ; // 默认为读

  if (flags & VFS_O_CREAT) {
    if (flags & VFS_O_EXCL)
      mode |= FA_CREATE_NEW;
    else if (flags & VFS_O_TRUNC)
      mode |= FA_CREATE_ALWAYS;
    else
      mode |= FA_OPEN_ALWAYS;
  } else {
    mode |= FA_OPEN_EXISTING;
  }

  if (flags & VFS_O_APPEND) {
    mode |= FA_OPEN_APPEND;
  }

  FRESULT res = f_open(fp, full_path, mode);
  if (res == FR_OK) {
    *file = (vfs_file_t)fp;
    return VFS_OK;
  }

#ifdef USE_MEMPOOL
  sys_free(FATFS_STRATEGY_MEMSOURCE, fp);
#else
  free(fp);
#endif
  return -res;
}

static int fatfs_vfs_read(vfs_file_t file, void *buf, size_t len) {
  FIL *fp = (FIL *)file;
  UINT bytes_read = 0;
  FRESULT res = f_read(fp, buf, len, &bytes_read);
  if (res == FR_OK)
    return bytes_read;
  return -res;
}

static int fatfs_vfs_write(vfs_file_t file, const void *buf, size_t len) {
  FIL *fp = (FIL *)file;
  UINT bytes_written = 0;
  FRESULT res = f_write(fp, buf, len, &bytes_written);
  if (res == FR_OK)
    return bytes_written;
  return -res;
}

static int fatfs_vfs_lseek(vfs_file_t file, off_t offset, int whence) {
  FIL *fp = (FIL *)file;
  FSIZE_t new_pos = 0;
  switch (whence) {
  case SEEK_SET:
    new_pos = offset;
    break;
  case SEEK_CUR:
    new_pos = f_tell(fp) + offset;
    break;
  case SEEK_END:
    new_pos = f_size(fp) + offset;
    break;
  default:
    return VFS_ERR_INVAL;
  }
  FRESULT res = f_lseek(fp, new_pos);
  return (res == FR_OK) ? new_pos : -res;
}

static int fatfs_vfs_sync(vfs_file_t file) {
  FIL *fp = (FIL *)file;
  FRESULT res = f_sync(fp);
  return (res == FR_OK) ? VFS_OK : -res;
}

static int fatfs_vfs_close(vfs_file_t file) {
  FIL *fp = (FIL *)file;
  FRESULT res = f_close(fp);
#ifdef USE_MEMPOOL
  sys_free(FATFS_STRATEGY_MEMSOURCE, fp);
#else
  free(fp);
#endif
  return (res == FR_OK) ? VFS_OK : -res;
}

static int fatfs_vfs_opendir(mount_point_t *mp, vfs_dir_t *dir,
                             const char *path) {
  fatfs_strategy_t *strat = (fatfs_strategy_t *)mp->fs_strategy;
#ifdef USE_MEMPOOL
  DIR *dp = sys_malloc(FATFS_STRATEGY_MEMSOURCE, sizeof(DIR));
#else
  DIR *dp = malloc(sizeof(DIR));
#endif
  if (!dp)
    return VFS_ERR_NO_MEM;

  char full_path[64];
  if (path[0] == '/')
    path++;
  snprintf(full_path, sizeof(full_path), "%s/%s", strat->drive_num, path);

  FRESULT res = f_opendir(dp, full_path);
  if (res == FR_OK) {
    *dir = (vfs_dir_t)dp;
    return VFS_OK;
  }
#ifdef USE_MEMPOOL
  sys_free(FATFS_STRATEGY_MEMSOURCE, dp);
#else
  free(dp);
#endif
  return -res;
}

static int fatfs_vfs_readdir(vfs_dir_t dir, void *dirent) {
  DIR *dp = (DIR *)dir;
  if (!dp || !dirent)
    return VFS_ERR_INVAL;

  vfs_dirent_t *v_dir = (vfs_dirent_t *)dirent;
  FILINFO fno;

  FRESULT res = f_readdir(dp, &fno);
  if (res == FR_OK && fno.fname[0] != 0) {
    strncpy(v_dir->name, fno.fname, sizeof(v_dir->name) - 1);
    v_dir->name[sizeof(v_dir->name) - 1] = '\0';
    v_dir->info.size = fno.fsize;
    v_dir->info.is_dir = (fno.fattrib & AM_DIR) ? true : false;
    return 1; // 成功读取一条目
  }
  return 0; // 到达目录末尾或发生错误
}

static int fatfs_vfs_closedir(vfs_dir_t dir) {
  DIR *dp = (DIR *)dir;
  if (!dp)
    return VFS_ERR_INVAL;
  FRESULT res = f_closedir(dp);
#ifdef USE_MEMPOOL
  sys_free(FATFS_STRATEGY_MEMSOURCE, dp);
#else
  free(dp);
#endif
  return (res == FR_OK) ? VFS_OK : -res;
}

static int fatfs_vfs_mkdir(mount_point_t *mp, const char *path) {
  fatfs_strategy_t *strat = (fatfs_strategy_t *)mp->fs_strategy;
  char full_path[64];
  if (path[0] == '/')
    path++;
  snprintf(full_path, sizeof(full_path), "%s/%s", strat->drive_num, path);
  FRESULT res = f_mkdir(full_path);
  return (res == FR_OK) ? VFS_OK : -res;
}

static int fatfs_vfs_unlink(mount_point_t *mp, const char *path) {
  fatfs_strategy_t *strat = (fatfs_strategy_t *)mp->fs_strategy;
  char full_path[64];
  if (path[0] == '/')
    path++;
  snprintf(full_path, sizeof(full_path), "%s/%s", strat->drive_num, path);
  FRESULT res = f_unlink(full_path);
  return (res == FR_OK) ? VFS_OK : -res;
}

static int fatfs_vfs_rename(mount_point_t *mp, const char *old_path,
                            const char *new_path) {
  fatfs_strategy_t *strat = (fatfs_strategy_t *)mp->fs_strategy;
  char full_old[64], full_new[64];
  if (old_path[0] == '/')
    old_path++;
  if (new_path[0] == '/')
    new_path++;
  snprintf(full_old, sizeof(full_old), "%s/%s", strat->drive_num, old_path);
  snprintf(full_new, sizeof(full_new), "%s/%s", strat->drive_num, new_path);
  FRESULT res = f_rename(full_old, full_new);
  return (res == FR_OK) ? VFS_OK : -res;
}

static int fatfs_vfs_stat(mount_point_t *mp, const char *path,
                          struct vfs_stat_t *st) {
  fatfs_strategy_t *strat = (fatfs_strategy_t *)mp->fs_strategy;
  char full_path[64];
  if (path[0] == '/')
    path++;
  snprintf(full_path, sizeof(full_path), "%s/%s", strat->drive_num, path);

  FILINFO fno;
  FRESULT res = f_stat(full_path, &fno);
  if (res == FR_OK && st) {
    st->size = fno.fsize;
    st->is_dir = (fno.fattrib & AM_DIR) ? true : false;
    return VFS_OK;
  }
  return -res;
}

static int fatfs_vfs_ioctl(mount_point_t *mp, int cmd, void *arg) {
  return VFS_ERR_GENERAL;
}

static const fs_ops_t fatfs_ops = {.mount = fatfs_vfs_mount,
                                   .unmount = fatfs_vfs_unmount,
                                   .format = fatfs_vfs_format,
                                   .open = fatfs_vfs_open,
                                   .read = fatfs_vfs_read,
                                   .write = fatfs_vfs_write,
                                   .lseek = fatfs_vfs_lseek,
                                   .sync = fatfs_vfs_sync,
                                   .close = fatfs_vfs_close,
                                   .opendir = fatfs_vfs_opendir,
                                   .readdir = fatfs_vfs_readdir,
                                   .closedir = fatfs_vfs_closedir,
                                   .mkdir = fatfs_vfs_mkdir,
                                   .unlink = fatfs_vfs_unlink,
                                   .rename = fatfs_vfs_rename,
                                   .stat = fatfs_vfs_stat,
                                   .ioctl = fatfs_vfs_ioctl};

fs_strategy_t *fatfs_strategy_create(void) {
#ifdef USE_MEMPOOL
  fatfs_strategy_t *fat_str =
      sys_malloc(FATFS_STRATEGY_MEMSOURCE, sizeof(fatfs_strategy_t));
#else
  fatfs_strategy_t *fat_str = malloc(sizeof(fatfs_strategy_t));
#endif

  if (fat_str) {
    memset(fat_str, 0, sizeof(fatfs_strategy_t));
    fat_str->base.ops = &fatfs_ops;
    // 默认盘符 0:
    fat_str->pdrv = 0;
    strcpy(fat_str->drive_num, "0:");
  }
  return (fs_strategy_t *)fat_str;
}

void fatfs_strategy_destroy(fatfs_strategy_t *strategy) {
  if (strategy) {
#ifdef USE_MEMPOOL
    sys_free(FATFS_STRATEGY_MEMSOURCE, strategy);
#else
    free(strategy);
#endif
  }
}
