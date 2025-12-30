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

// QSPI Base Address for XIP (Typically 0x90000000)
#define QSPI_MAP_ADDR 0x90000000

// Resource Partition Offset (e.g. 1MB reserved for boot/config)
#define RES_PARTITION_OFFSET 0x100000

static const res_map_t res_table[] = {
    // Offset calculation: previous_offset + previous_size (aligned to 4 bytes
    // ideally)
    // Note: These need to match exactly with res_burner.c's write logic
    {RES_IMG_WALLPAPER, 0x000000, 307200 + 4,
     "Wallpaper"}, // 480*320*2 + 4 header
    {RES_IMG_ICON_WIFI, 0x050000, 5000 + 4, "WiFi Icon"}, // Placeholder size
    {RES_IMG_ICON_BRIGHT, 0x052000, 5000 + 4, "Brightness Icon"},
    {RES_IMG_ICON_COLORWHEEL, 0x054000, 5000 + 4, "Colorwheel Icon"},
    {RES_IMG_ICON_CONTROL, 0x056000, 5000 + 4, "Control Icon"},
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
