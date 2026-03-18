/*
 * lvgl_resource.h
 *
 *  Created on: Mar 17, 2026
 *      Author: 12114
 */

#ifndef LVGL_RESOURCE_LVGL_RESOURCE_H_
#define LVGL_RESOURCE_LVGL_RESOURCE_H_

#include "Project_cfg.h"
#include "lvgl.h"
#include "asset_manager.h"

// 烧录模式 (编译图像 C 数组用于烧写)
// 运行模式 (图像数组不编译，零占用，全靠外部 NorFlash 动态读取)
#ifdef RES_BURN_ENABLE
#define RES_BURN_MODE 1
#endif
/**
 * @brief 所有资源的ID
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
  RES_COUNT_MAX
} res_id_t;

/**
 * @brief 初始化资源管理 (读取内部表并尝试对接外部 Flash)
 * @return int 0:成功
 */
int res_init(void);

/**
 * @brief 获取资源
 */
const lv_img_dsc_t *res_get_img(res_id_t id);
const lv_font_t *res_get_font(res_id_t id);
const void *res_get_audio(res_id_t id);

#if RES_BURN_MODE
/**
 * @brief 执行资产全量烧录到外部 NOR Flash
 * @return int 0:成功
 */
int res_program_all(void);
#endif

#endif /* LVGL_RESOURCE_LVGL_RESOURCE_H_ */
