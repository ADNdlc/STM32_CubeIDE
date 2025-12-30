#include "res_manager.h"
#include "elog.h"
#include <stddef.h>

#define LOG_TAG "RES_MGR"

typedef struct {
  res_id_t id;
  uint32_t offset;
  uint32_t size;
  const char *desc;
} res_map_t;

// QSPI Base Address & Offset now in header
// static const res_map_t res_table[] ....

static const res_map_t res_table[] = {
    // Offset calculation: must match res_burner.c logic
    {RES_IMG_WALLPAPER, RES_OFFSET_WALLPAPER, 307200 + 4, "Wallpaper"},
    {RES_IMG_ICON_WIFI, RES_OFFSET_ICON_WIFI, 5000 + 4, "WiFi Icon"},
    {RES_IMG_ICON_BRIGHT, RES_OFFSET_ICON_BRIGHT, 5000 + 4, "Brightness Icon"},
    {RES_IMG_ICON_COLORWHEEL, RES_OFFSET_ICON_COLORWHEEL, 5000 + 4,
     "Colorwheel Icon"},
    {RES_IMG_ICON_CONTROL, RES_OFFSET_ICON_CONTROL, 5000 + 4, "Control Icon"},
};

const void *res_get_src(res_id_t id) {
  if (id >= RES_IMG_COUNT) {
    log_e("Invalid Resource ID: %d", id);
    return NULL;
  }

  for (size_t i = 0; i < sizeof(res_table) / sizeof(res_table[0]); i++) {
    if (res_table[i].id == id) {
      uint32_t addr =
          QSPI_MAP_ADDR + RES_PARTITION_OFFSET + res_table[i].offset;
      log_d("Loading resource: %s (Addr: 0x%08X)", res_table[i].desc, addr);
      return (const void *)addr;
    }
  }

  log_w("Resource ID %d not found in map", id);
  return NULL;
}
