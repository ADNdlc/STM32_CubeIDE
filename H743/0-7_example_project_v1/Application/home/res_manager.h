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

/**
 * @brief 获取资源
 *
 * @param id Resource ID
 * @return const void* LVGL资源: FS路径 (外部flash) or 地址 (片上)
 */
const void *res_get_src(res_id_t id);

#endif /* APPLICATION_HOME_RES_MANAGER_H_ */
