#include "fatfs_strategy.h"
#include "../vfs_manager.h"
#include "ff.h"
#include "MemPool.h"
#include "elog.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#define LOG_TAG "FATFS_STRAT"
#define FATFS_STRATEGY_MEMSOURCE SYS_MEM_INTERNAL

static const TCHAR _fatfs_drive[] = "0:";

static int fatfs_vfs_mount(mount_point_t *mp) {
    // 从 mp->fs_strategy 拿不到 fatfs_strategy_t 的直接指针
    // 通过 offsetof 转换，或者用单独的指针
    // 假设挂载成功。真正的 f_mount 需要一个外部挂载点。
    fatfs_strategy_t *strat = (fatfs_strategy_t *)((uint8_t*)mp->fs_strategy - offsetof(fatfs_strategy_t, Base));
    
    FRESULT res = f_mount(&strat->fs, _fatfs_drive, 1);
    return (res == FR_OK) ? 0 : -1;
}

static int fatfs_vfs_unmount(mount_point_t *mp) {
    FRESULT res = f_mount(NULL, _fatfs_drive, 0);
    return (res == FR_OK) ? 0 : -1;
}

static int fatfs_vfs_format(mount_point_t *mp) {
//    BYTE work[_MAX_SS];
//    FRESULT res = f_mkfs(_fatfs_drive, FM_ANY, 0, work, sizeof(work));
//    return (res == FR_OK) ? 0 : -1;
}

static int fatfs_vfs_open(mount_point_t *mp, vfs_file_t *file, const char *path, int flags) {
    FIL *fp = malloc(sizeof(FIL));
    if (!fp) return -1;
    char full_path[64];
    snprintf(full_path, sizeof(full_path), "%s/%s", _fatfs_drive, path);
    // 忽略特定 flags，统一按照读写创建处理
    BYTE mode = FA_READ | FA_WRITE | FA_OPEN_ALWAYS; 
    FRESULT res = f_open(fp, full_path, mode);
    if (res == FR_OK) {
        *file = (vfs_file_t)fp;
        return 0;
    }
    free(fp);
    return -1;
}

static int fatfs_vfs_read(vfs_file_t file, void *buf, size_t len) {
    FIL *fp = (FIL *)file;
    UINT br;
    FRESULT res = f_read(fp, buf, len, &br);
    return (res == FR_OK) ? (int)br : -1;
}

static int fatfs_vfs_write(vfs_file_t file, const void *buf, size_t len) {
    FIL *fp = (FIL *)file;
    UINT bw;
    FRESULT res = f_write(fp, buf, len, &bw);
    return (res == FR_OK) ? (int)bw : -1;
}

static int fatfs_vfs_lseek(vfs_file_t file, off_t offset, int whence) {
    FIL *fp = (FIL *)file;
    FSIZE_t abs_offset = offset; // SEEK_SET
    // 在本实现中假设 offset 已经是绝对位置。如果需要完整 POSIX
    // 需要根据 whence（通常 0=SEEK_SET, 1=SEEK_CUR, 2=SEEK_END）计算。
    if (whence == 1) abs_offset = f_tell(fp) + offset;
    else if (whence == 2) abs_offset = f_size(fp) + offset;
    FRESULT res = f_lseek(fp, abs_offset);
    return (res == FR_OK) ? 0 : -1;
}

static int fatfs_vfs_sync(vfs_file_t file) {
    FIL *fp = (FIL *)file;
    return (f_sync(fp) == FR_OK) ? 0 : -1;
}

static int fatfs_vfs_close(vfs_file_t file) {
    FIL *fp = (FIL *)file;
    int res = f_close(fp);
    free(fp);
    return (res == FR_OK) ? 0 : -1;
}

static int fatfs_vfs_opendir(mount_point_t *mp, vfs_dir_t *dir, const char *path) {
    DIR *dp = malloc(sizeof(DIR));
    if (!dp) return -1;
    char full_path[64];
    snprintf(full_path, sizeof(full_path), "%s/%s", _fatfs_drive, path);
    FRESULT res = f_opendir(dp, full_path);
    if (res == FR_OK) {
        *dir = (vfs_dir_t)dp;
        return 0;
    }
    free(dp);
    return -1;
}

static int fatfs_vfs_readdir(vfs_dir_t dir, void *dirent) {
    DIR *dp = (DIR *)dir;
    FILINFO *fno = (FILINFO *)dirent;
    FRESULT res = f_readdir(dp, fno);
    return (res == FR_OK && fno->fname[0] != 0) ? 1 : 0;
}

static int fatfs_vfs_closedir(vfs_dir_t dir) {
    DIR *dp = (DIR *)dir;
    int res = f_closedir(dp);
    free(dp);
    return (res == FR_OK) ? 0 : -1;
}

static int fatfs_vfs_mkdir(mount_point_t *mp, const char *path) {
    char full_path[64];
    snprintf(full_path, sizeof(full_path), "%s/%s", _fatfs_drive, path);
    return (f_mkdir(full_path) == FR_OK) ? 0 : -1;
}

static int fatfs_vfs_unlink(mount_point_t *mp, const char *path) {
    char full_path[64];
    snprintf(full_path, sizeof(full_path), "%s/%s", _fatfs_drive, path);
    return (f_unlink(full_path) == FR_OK) ? 0 : -1;
}

static int fatfs_vfs_rename(mount_point_t *mp, const char *old_path, const char *new_path) {
    char f_old[64], f_new[64];
    snprintf(f_old, sizeof(f_old), "%s/%s", _fatfs_drive, old_path);
    snprintf(f_new, sizeof(f_new), "%s/%s", _fatfs_drive, new_path);
    return (f_rename(f_old, f_new) == FR_OK) ? 0 : -1;
}

static int fatfs_vfs_stat(mount_point_t *mp, const char *path, struct vfs_stat_t *st) {
    char full_path[64];
    snprintf(full_path, sizeof(full_path), "%s/%s", _fatfs_drive, path);
    FILINFO fno;
    FRESULT res = f_stat(full_path, &fno);
    if (res == FR_OK && st) {
        st->size = fno.fsize;
        st->is_dir = (fno.fattrib & AM_DIR) != 0;
    }
    return (res == FR_OK) ? 0 : -1;
}

static int fatfs_vfs_ioctl(mount_point_t *mp, int cmd, void *arg) {
    return -1;
}

static const fs_ops_t fatfs_ops = {
    .mount = fatfs_vfs_mount,
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
    .ioctl = fatfs_vfs_ioctl
};

fs_strategy_t *fatfs_strategy_create(void) {
    fatfs_strategy_t *strat = malloc(sizeof(fatfs_strategy_t));
    if (!strat) return NULL;
    memset(strat, 0, sizeof(fatfs_strategy_t));
    
    strat->Base = malloc(sizeof(fs_strategy_t));
    if (!strat->Base) {
        free(strat);
        return NULL;
    }
    
    strat->Base->ops = &fatfs_ops;
    strat->Base->name = "fatfs";
    
    return strat->Base;
}

void fatfs_strategy_destroy(fatfs_strategy_t *strategy) {
    if (strategy) {
        if (strategy->Base) free(strategy->Base);
        free(strategy);
    }
}
