/*
 * lvgl_resource.h
 *
 *  Created on: Mar 17, 2026
 *      Author: 12114
 */

#ifndef LVGL_RESOURCE_LVGL_RESOURCE_H_
#define LVGL_RESOURCE_LVGL_RESOURCE_H_

#include "asset_manager.h"

/**
 * @brief Resource ID for all resources
 */
typedef enum {
  RES_IMG_TEST = 0,

  RES_IMG_WALLPAPER,
  RES_IMG_ICON_WIFI,
  RES_IMG_ICON_BRIGHT,
  RES_IMG_ICON_COLORWHEEL,
  RES_IMG_ICON_CONTROL,
  RES_IMG_DEFAULT_USER,
  RES_IMG_IMG_LIGHT,
  //...
  RES_IMG_COUNT_MAX
} lvgl_res_id_t;

/**
 * @brief 获取资源(无论内外统一使用此api)
 *
 * @param id Resource ID
 * @return const void* LVGL资源: FS路径 (外部flash) or 地址 (片上或norflash)
 */
const void *res_get_src(lvgl_res_id_t id);

#endif /* LVGL_RESOURCE_LVGL_RESOURCE_H_ */
