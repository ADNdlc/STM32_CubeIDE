#include "res_manager.h"
#include "elog.h"
#include <stddef.h>

#define LOG_TAG "RES_MGR"

typedef struct {
  res_id_t id;
  const char *path;
  const char *desc;
} res_map_t;

static const res_map_t res_table[] = {
    {RES_IMG_WALLPAPER, "L:home/wallpaper.bin", "Wallpaper"},
    {RES_IMG_ICON_WIFI, "L:sys/wifi.bin", "WiFi Icon"},
    {RES_IMG_ICON_BRIGHT, "L:sys/bright.bin", "Brightness Icon"},
    {RES_IMG_ICON_COLORWHEEL, "L:apps/colorwheel.bin", "Colorwheel Icon"},
    {RES_IMG_ICON_CONTROL, "L:apps/control.bin", "Control Icon"},
};

const char *res_get_src(res_id_t id) {
  if (id >= RES_IMG_COUNT) {
    log_e("Invalid Resource ID: %d", id);
    return NULL;
  }

  for (size_t i = 0; i < sizeof(res_table) / sizeof(res_table[0]); i++) {
    if (res_table[i].id == id) {
      log_d("Loading resource: %s (%s)", res_table[i].desc, res_table[i].path);
      return res_table[i].path;
    }
  }

  log_w("Resource ID %d not found in map", id);
  return NULL;
}
