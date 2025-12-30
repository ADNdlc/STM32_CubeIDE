#ifndef APPLICATION_HOME_RES_MANAGER_H_
#define APPLICATION_HOME_RES_MANAGER_H_

#include <stdint.h>

/**
 * @brief Resource ID for all external resources
 */
typedef enum {
  RES_IMG_WALLPAPER = 0,
  RES_IMG_ICON_WIFI,
  RES_IMG_ICON_BRIGHT,
  RES_IMG_ICON_COLORWHEEL,
  RES_IMG_ICON_CONTROL,
  RES_IMG_COUNT
} res_id_t;

// XIP Mapping Configuration
#define QSPI_MAP_ADDR 0x90000000
#define RES_PARTITION_OFFSET 0x100000 // 1MB reserved

// Resource Offsets (Physical relative to Partition Start)
#define RES_OFFSET_WALLPAPER 0x000000
#define RES_OFFSET_ICON_WIFI 0x050000
#define RES_OFFSET_ICON_BRIGHT 0x052000
#define RES_OFFSET_ICON_COLORWHEEL 0x054000
#define RES_OFFSET_ICON_CONTROL 0x056000

/**
 * @brief Get the source path for an image resource
 *
 * @param id Resource ID
 * @return const char* LVGL source path (e.g. "L:home/wallpaper.bin")
 */
const void *res_get_src(res_id_t id);

#endif /* APPLICATION_HOME_RES_MANAGER_H_ */
