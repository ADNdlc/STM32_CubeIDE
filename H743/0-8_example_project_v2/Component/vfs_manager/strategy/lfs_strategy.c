#include "lfs_strategy.h"
#include "vfs_manager.h"
#include <stdlib.h>
#include <string.h>

// Glue functions for LittleFS
static int lfs_glue_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
    mount_point_t *mp = (mount_point_t *)c->context;
    uint32_t addr = block * c->block_size + off;
    return (STORAGE_READ(mp->device, addr, buffer, size) == 0) ? LFS_ERR_OK : LFS_ERR_IO;
}

static int lfs_glue_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    mount_point_t *mp = (mount_point_t *)c->context;
    uint32_t addr = block * c->block_size + off;
    return (STORAGE_WRITE(mp->device, addr, (uint8_t*)buffer, size) == 0) ? LFS_ERR_OK : LFS_ERR_IO;
}

static int lfs_glue_erase(const struct lfs_config *c, lfs_block_t block) {
    mount_point_t *mp = (mount_point_t *)c->context;
    uint32_t addr = block * c->block_size;
    return (STORAGE_ERASE(mp->device, addr, c->block_size) == 0) ? LFS_ERR_OK : LFS_ERR_IO;
}

static int lfs_glue_sync(const struct lfs_config *c) {
    mount_point_t *mp = (mount_point_t *)c->context;
    STORAGE_CONTROL(mp->device, 0 /* CMD_SYNC */, NULL);
    return LFS_ERR_OK;
}

// VFS Strategy Ops implementation
static int lfs_vfs_mount(mount_point_t *mp) {
    lfs_strategy_t *lfs_strat = (lfs_strategy_t *)mp->fs_strategy;
    storage_info_t info;
    STORAGE_GET_INFO(mp->device, &info);

    // 设置 LFS 配置
    lfs_strat->cfg.context = mp;
    lfs_strat->cfg.read = lfs_glue_read;
    lfs_strat->cfg.prog = lfs_glue_prog;
    lfs_strat->cfg.erase = lfs_glue_erase;
    lfs_strat->cfg.sync = lfs_glue_sync;

    lfs_strat->cfg.block_size = info.erase_size;
    lfs_strat->cfg.block_count = info.total_size / info.erase_size;
    lfs_strat->cfg.lookahead_size = 32;
    lfs_strat->cfg.cache_size = info.erase_size > 256 ? 256 : info.erase_size;
    lfs_strat->cfg.block_cycles = 500;

    int err = lfs_mount(&lfs_strat->lfs, &lfs_strat->cfg);
    if (err) {
        // 如果挂载失败，尝试格式化 (可选，视需求而定)
        // err = lfs_format(&lfs_strat->lfs, &lfs_strat->cfg);
        // if (err == 0) err = lfs_mount(&lfs_strat->lfs, &lfs_strat->cfg);
    }
    return err;
}

static int lfs_vfs_unmount(mount_point_t *mp) {
    lfs_strategy_t *lfs_strat = (lfs_strategy_t *)mp->fs_strategy;
    return lfs_unmount(&lfs_strat->lfs);
}

typedef struct {
    lfs_t *lfs;
    lfs_file_t file;
} lfs_vfs_file_t;

typedef struct {
    lfs_t *lfs;
    struct lfs_dir dir;
} lfs_vfs_dir_t;

static int lfs_vfs_open(mount_point_t *mp, vfs_file_t *file, const char *path, int flags) {
    lfs_strategy_t *lfs_strat = (lfs_strategy_t *)mp->fs_strategy;
    lfs_vfs_file_t *lvf = malloc(sizeof(lfs_vfs_file_t));
    if (!lvf) return -1;

    lvf->lfs = &lfs_strat->lfs;
    int lfs_flags = LFS_O_RDWR | LFS_O_CREAT; 
    
    int err = lfs_file_open(lvf->lfs, &lvf->file, path, lfs_flags);
    if (err == 0) {
        *file = (vfs_file_t)lvf;
        return 0;
    }
    free(lvf);
    return err;
}

static int lfs_vfs_read(vfs_file_t file, void *buf, size_t len) {
    lfs_vfs_file_t *lvf = (lfs_vfs_file_t *)file;
    return lfs_file_read(lvf->lfs, &lvf->file, buf, len);
}

static int lfs_vfs_write(vfs_file_t file, const void *buf, size_t len) {
    lfs_vfs_file_t *lvf = (lfs_vfs_file_t *)file;
    return lfs_file_write(lvf->lfs, &lvf->file, buf, len);
}

static int lfs_vfs_close(vfs_file_t file) {
    lfs_vfs_file_t *lvf = (lfs_vfs_file_t *)file;
    int res = lfs_file_close(lvf->lfs, &lvf->file);
    free(lvf);
    return res;
}

static int lfs_vfs_opendir(mount_point_t *mp, vfs_dir_t *dir, const char *path) {
    lfs_strategy_t *lfs_strat = (lfs_strategy_t *)mp->fs_strategy;
    lfs_vfs_dir_t *lvd = malloc(sizeof(lfs_vfs_dir_t));
    if (!lvd) return -1;

    lvd->lfs = &lfs_strat->lfs;
    int err = lfs_dir_open(lvd->lfs, &lvd->dir, path);
    if (err == 0) {
        *dir = (vfs_dir_t)lvd;
        return 0;
    }
    free(lvd);
    return err;
}

static int lfs_vfs_readdir(vfs_dir_t dir, void *dirent) {
    lfs_vfs_dir_t *lvd = (lfs_vfs_dir_t *)dir;
    return lfs_dir_read(lvd->lfs, &lvd->dir, (struct lfs_info *)dirent);
}

static int lfs_vfs_closedir(vfs_dir_t dir) {
    lfs_vfs_dir_t *lvd = (lfs_vfs_dir_t *)dir;
    int res = lfs_dir_close(lvd->lfs, &lvd->dir);
    free(lvd);
    return res;
}

static int lfs_vfs_format(mount_point_t *mp) {
    lfs_strategy_t *lfs_strat = (lfs_strategy_t *)mp->fs_strategy;
    return lfs_format(&lfs_strat->lfs, &lfs_strat->cfg);
}

static int lfs_vfs_lseek(vfs_file_t file, off_t offset, int whence) {
    lfs_vfs_file_t *lvf = (lfs_vfs_file_t *)file;
    return lfs_file_seek(lvf->lfs, &lvf->file, offset, whence);
}

static int lfs_vfs_sync(vfs_file_t file) {
    lfs_vfs_file_t *lvf = (lfs_vfs_file_t *)file;
    return lfs_file_sync(lvf->lfs, &lvf->file);
}

static int lfs_vfs_mkdir(mount_point_t *mp, const char *path) {
    lfs_strategy_t *lfs_strat = (lfs_strategy_t *)mp->fs_strategy;
    return lfs_mkdir(&lfs_strat->lfs, path);
}

static int lfs_vfs_unlink(mount_point_t *mp, const char *path) {
    lfs_strategy_t *lfs_strat = (lfs_strategy_t *)mp->fs_strategy;
    return lfs_remove(&lfs_strat->lfs, path);
}

static int lfs_vfs_rename(mount_point_t *mp, const char *old_path, const char *new_path) {
    lfs_strategy_t *lfs_strat = (lfs_strategy_t *)mp->fs_strategy;
    return lfs_rename(&lfs_strat->lfs, old_path, new_path);
}

static int lfs_vfs_stat(mount_point_t *mp, const char *path, struct vfs_stat_t *st) {
    lfs_strategy_t *lfs_strat = (lfs_strategy_t *)mp->fs_strategy;
    struct lfs_info info;
    int err = lfs_stat(&lfs_strat->lfs, path, &info);
    if (err == 0 && st) {
        st->size = info.size;
        st->is_dir = (info.type == LFS_TYPE_DIR);
    }
    return err;
}

static int lfs_vfs_ioctl(mount_point_t *mp, int cmd, void *arg) {
    return -1; // 未支持
}

static const fs_ops_t s_lfs_ops = {
    .mount = lfs_vfs_mount,
    .unmount = lfs_vfs_unmount,
    .format = lfs_vfs_format,
    .open = lfs_vfs_open,
    .read = lfs_vfs_read,
    .write = lfs_vfs_write,
    .lseek = lfs_vfs_lseek,
    .sync = lfs_vfs_sync,
    .close = lfs_vfs_close,
    .opendir = lfs_vfs_opendir,
    .readdir = lfs_vfs_readdir,
    .closedir = lfs_vfs_closedir,
    .mkdir = lfs_vfs_mkdir,
    .unlink = lfs_vfs_unlink,
    .rename = lfs_vfs_rename,
    .stat = lfs_vfs_stat,
    .ioctl = lfs_vfs_ioctl
};

fs_strategy_t *lfs_strategy_create(const lfs_strategy_config_t *config) {
    lfs_strategy_t *strat = malloc(sizeof(lfs_strategy_t));
    if (!strat) return NULL;
    memset(strat, 0, sizeof(lfs_strategy_t));
    
    strat->Base = malloc(sizeof(fs_strategy_t));
    strat->Base->ops = &s_lfs_ops;
    strat->Base->name = "lfs";
    
    return strat->Base;
}
