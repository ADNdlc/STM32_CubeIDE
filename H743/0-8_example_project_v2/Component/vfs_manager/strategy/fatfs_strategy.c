#include "fatfs_strategy.h"
#include "../vfs_manager.h"
#include "diskio.h"
#include "ff.h"


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
    return 0; // 挂载成功
  } else {
    // 如果挂载失败，注销 diskio 映射以防空指针误用
    fatfs_diskio_register(strat->pdrv, NULL);
    return -1;
  }
}

static int fatfs_vfs_unmount(mount_point_t *mp) {}

static int fatfs_vfs_format(mount_point_t *mp) {}

static int fatfs_vfs_open(mount_point_t *mp, vfs_file_t *file, const char *path, int flags) {}

static int fatfs_vfs_read(vfs_file_t file, void *buf, size_t len) {}

static int fatfs_vfs_write(vfs_file_t file, const void *buf, size_t len) {}

static int fatfs_vfs_lseek(vfs_file_t file, off_t offset, int whence) {}

static int fatfs_vfs_sync(vfs_file_t file) {}

static int fatfs_vfs_close(vfs_file_t file) {}

static int fatfs_vfs_opendir(mount_point_t *mp, vfs_dir_t *dir, const char *path) {}

static int fatfs_vfs_readdir(vfs_dir_t dir, void *dirent) {}

static int fatfs_vfs_closedir(vfs_dir_t dir) {}

static int fatfs_vfs_mkdir(mount_point_t *mp, const char *path) {}

static int fatfs_vfs_unlink(mount_point_t *mp, const char *path) {}

static int fatfs_vfs_rename(mount_point_t *mp, const char *old_path, const char *new_path) {}

static int fatfs_vfs_stat(mount_point_t *mp, const char *path, struct vfs_stat_t *st) {}

static int fatfs_vfs_ioctl(mount_point_t *mp, int cmd, void *arg) {}

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
}

void fatfs_strategy_destroy(fatfs_strategy_t *strategy) {
  if (strategy) {
  }
}
