/**
 * @file lv_port_fs.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_fs.h"
#include "elog.h"
#include "flash_handler.h"
#include "lfs.h"
#include "lvgl.h"
#include "strategy/lfs_strategy.h"
#include "sys.h"
#include <string.h>

#define LOG_TAG "LV_PORT_FS"

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void fs_init(void);

static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode);
static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p);
static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf,
                           uint32_t btr, uint32_t *br);
static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos,
                           lv_fs_whence_t whence);
static lv_fs_res_t fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p);

/**********************
 *  STATIC VARIABLES
 **********************/
// We need access to the lfs instance.
// For now, we assume the first LittleFS partition in flash_handler is our
// primary one. Historically, lfs_strategy_impl_t holds the lfs_t. A better way
// is to expose the lfs_t from the strategy if possible, or just re-open? Let's
// assume we can get it or at least call a helper.
extern lfs_t *lfs_strategy_get_lfs(flash_strategy_t *strat);

static lfs_t *g_lfs = NULL;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_fs_init(void) {
  fs_init();

  static lv_fs_drv_t fs_drv;
  lv_fs_drv_init(&fs_drv);

  fs_drv.letter = 'L';
  fs_drv.open_cb = fs_open;
  fs_drv.close_cb = fs_close;
  fs_drv.read_cb = fs_read;
  fs_drv.seek_cb = fs_seek;
  fs_drv.tell_cb = fs_tell;

  lv_fs_drv_register(&fs_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void fs_init(void) {
  // flash_handler should be initialized already.
  // We need to find the LFS strategy to get the lfs_t handle.
  // flash_handler doesn't expose strategy yet by prefix, but we can look it up
  // in future. For now, we'll implement a helper in flash_handler to find the
  // lfs_t.
  extern lfs_t *flash_handler_get_lfs(const char *prefix);
  g_lfs = flash_handler_get_lfs("/lfs");

  if (!g_lfs) {
    log_e("Failed to find LittleFS handle for LVGL");
  } else {
    log_i("LVGL FS bridge linked to /lfs successfully");
  }
}

static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode) {
  (void)drv;
  if (!g_lfs)
    return NULL;

  lfs_file_t *file =
      (lfs_file_t *)sys_malloc(SYS_MEM_INTERNAL, sizeof(lfs_file_t));
  if (!file)
    return NULL;

  int lfs_flags = 0;
  if (mode == LV_FS_MODE_WR)
    lfs_flags = LFS_O_WRONLY | LFS_O_CREAT;
  else if (mode == LV_FS_MODE_RD)
    lfs_flags = LFS_O_RDONLY;
  else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD))
    lfs_flags = LFS_O_RDWR | LFS_O_CREAT;

  int err = lfs_file_open(g_lfs, file, path, lfs_flags);
  if (err < 0) {
    log_w("LV FS: Failed to open %s (err: %d)", path, err);
    sys_free(SYS_MEM_INTERNAL, file);
    return NULL;
  }

  return (void *)file;
}

static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p) {
  (void)drv;
  if (!g_lfs || !file_p)
    return LV_FS_RES_INV_PARAM;

  lfs_file_t *file = (lfs_file_t *)file_p;
  lfs_file_close(g_lfs, file);
  sys_free(SYS_MEM_INTERNAL, file);

  return LV_FS_RES_OK;
}

static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf,
                           uint32_t btr, uint32_t *br) {
  (void)drv;
  if (!g_lfs || !file_p)
    return LV_FS_RES_INV_PARAM;

  lfs_file_t *file = (lfs_file_t *)file_p;
  lfs_ssize_t res = lfs_file_read(g_lfs, file, buf, btr);

  if (res < 0)
    return LV_FS_RES_FS_ERR;

  if (br)
    *br = (uint32_t)res;
  return LV_FS_RES_OK;
}

static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos,
                           lv_fs_whence_t whence) {
  (void)drv;
  if (!g_lfs || !file_p)
    return LV_FS_RES_INV_PARAM;

  lfs_file_t *file = (lfs_file_t *)file_p;
  int lfs_whence = LFS_SEEK_SET;
  if (whence == LV_FS_SEEK_CUR)
    lfs_whence = LFS_SEEK_CUR;
  else if (whence == LV_FS_SEEK_END)
    lfs_whence = LFS_SEEK_END;

  lfs_soff_t res = lfs_file_seek(g_lfs, file, pos, lfs_whence);

  return (res >= 0) ? LV_FS_RES_OK : LV_FS_RES_FS_ERR;
}

static lv_fs_res_t fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p) {
  (void)drv;
  if (!g_lfs || !file_p || !pos_p)
    return LV_FS_RES_INV_PARAM;

  lfs_file_t *file = (lfs_file_t *)file_p;
  lfs_soff_t res = lfs_file_tell(g_lfs, file);

  if (res < 0)
    return LV_FS_RES_FS_ERR;

  *pos_p = (uint32_t)res;
  return LV_FS_RES_OK;
}
