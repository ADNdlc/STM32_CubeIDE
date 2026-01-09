#include "res_burner.h"
#include "elog.h"
#include "flash_handler.h"
#include "lfs.h"
#include "lvgl.h"
#include "res_manager.h"
#include "sys.h"
#include <string.h>


#define LOG_TAG "RES_BURNER"

// 需要包含所有使用到的资源包括图片和字库
LV_IMG_DECLARE(wallpaper);
LV_IMG_DECLARE(icon_wifi);
LV_IMG_DECLARE(icon_bright);
LV_IMG_DECLARE(icon_colorwheel);
LV_IMG_DECLARE(icon_Contol);

// 资源项
typedef struct {
  const char *path;          // 资源在文件系统中的路径
  const lv_img_dsc_t *dsc;   // 指向资源数据结构的指针
} burn_item_t;

// 资源列表
static const burn_item_t burn_list[] = {
    {"home/wallpaper.bin", &wallpaper},
    {"sys/wifi.bin", &icon_wifi},
    {"sys/bright.bin", &icon_bright},
    {"apps/colorwheel.bin", &icon_colorwheel},
    {"apps/control.bin", &icon_Contol},
};

/**
 * @brief 检查并创建目录
 * 
 * @param lfs   文件系统句柄
 * @param path  目录路径
 * @return int  0 成功，负值失败
 */
static int ensure_dir(lfs_t *lfs, const char *path) {
  char tmp[64];
  strncpy(tmp, path, sizeof(tmp));
  char *last_slash = strrchr(tmp, '/');
  if (last_slash) {
    *last_slash = '\0';
    // Simple one-level mkdir for now
    int err = lfs_mkdir(lfs, tmp);
    if (err < 0 && err != LFS_ERR_EXIST)
      return err;
  }
  return 0;
}

void res_burner_run(void) {
  log_i("Resource Burner: Starting...");

  lfs_t *lfs = flash_handler_get_lfs("/lfs");  // 获取 /lfs 挂载点的 LittleFS 句柄
  if (!lfs) {
    log_e("Burner: LittleFS handle not found!");
    return;
  }

  // 烧录所有资源
  for (size_t i = 0; i < sizeof(burn_list) / sizeof(burn_list[0]); i++) {
    const burn_item_t *item = &burn_list[i];

    log_i("Burning %s...", item->path);
    ensure_dir(lfs, item->path);

    lfs_file_t file;
    int err = lfs_file_open(lfs, &file, item->path,
                            LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
    if (err < 0) {
      log_e("Failed to open %s for burning (%d)", item->path, err);
      continue;
    }

    // 1. Write LVGL Image Header (4 bytes)
    lfs_file_write(lfs, &file, &item->dsc->header, sizeof(item->dsc->header));

    // 2. Write Image Data
    lfs_file_write(lfs, &file, item->dsc->data, item->dsc->data_size);

    lfs_file_close(lfs, &file);
    log_i("Burned %s success (%d bytes)", item->path, item->dsc->data_size + 4);
  }

  log_i("Resource Burner: All resources written.");
}
