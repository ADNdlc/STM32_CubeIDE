#include "res_manager.h"
#include "elog.h"
#include <stddef.h>

#define LOG_TAG "RES_MGR"

#define RES_USE_INTERNAL_FLASH 1		// 将资源数据编译到片上flash

#if RES_USE_INTERNAL_FLASH
#include "lvgl.h"

// Declare internal image resources
LV_IMG_DECLARE(wallpaper);
LV_IMG_DECLARE(icon_wifi);
LV_IMG_DECLARE(icon_bright);
LV_IMG_DECLARE(icon_colorwheel);
LV_IMG_DECLARE(icon_Contol);
#endif

typedef struct {
  res_id_t id;
  const void *src;
  const char *desc;
} res_map_t;

static const res_map_t res_table[] = {
#if RES_USE_INTERNAL_FLASH
    {RES_IMG_WALLPAPER, &wallpaper, "Wallpaper"},
    {RES_IMG_ICON_WIFI, &icon_wifi, "WiFi Icon"},
    {RES_IMG_ICON_BRIGHT, &icon_bright, "Brightness Icon"},
    {RES_IMG_ICON_COLORWHEEL, &icon_colorwheel, "Colorwheel Icon"},
    {RES_IMG_ICON_CONTROL, &icon_Contol, "Control Icon"},
#else
    {RES_IMG_WALLPAPER, "L:home/wallpaper.bin", "Wallpaper"},
    {RES_IMG_ICON_WIFI, "L:sys/icon_wifi.bin", "WiFi Icon"},
    {RES_IMG_ICON_BRIGHT, "L:sys/icon_bright.bin", "Brightness Icon"},
    {RES_IMG_ICON_COLORWHEEL, "L:apps/icon_colorwheel.bin", "Colorwheel Icon"},
    {RES_IMG_ICON_CONTROL, "L:apps/icon_Contol.bin", "Control Icon"},
#endif
};

const void *res_get_src(res_id_t id) {
  if (id >= RES_IMG_COUNT) {
    log_e("Invalid Resource ID: %d", id);
    return NULL;
  }

  for (size_t i = 0; i < sizeof(res_table) / sizeof(res_table[0]); i++) {
    if (res_table[i].id == id) {
#if RES_USE_INTERNAL_FLASH
      log_d("Loading internal resource: %s", res_table[i].desc);
#else
      log_d("Loading external resource: %s (%s)", res_table[i].desc,
            (const char *)res_table[i].src);
#endif
      return res_table[i].src;
    }
  }

  log_w("Resource ID %d not found in map", id);
  return NULL;
}
