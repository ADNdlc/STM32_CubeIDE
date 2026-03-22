/*
 * lvgl_resource.c
 *
 *  Created on: Mar 17, 2026
 *      Author: 12114
 */

#ifndef LVGL_RESOURCE_LVGL_RESOURCE_C_
#define LVGL_RESOURCE_LVGL_RESOURCE_C_

#include "lvgl_resource.h"
#include "Project_cfg.h"
#include "Sys.h"
#include "asset_manager.h"
#include "elog.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define LOG_TAG "LV_RES"


// 各种类型的影子描述符表
static lv_img_dsc_t s_img_dscs[RES_COUNT_MAX];
static lv_font_t s_font_dscs[RES_COUNT_MAX];
static const void *s_audio_ptrs[RES_COUNT_MAX];

/* --------- 仅烧录模式或测试显示模式编译图片数据 --------- */
/* --- 编译控制 --- */
#if RES_BURN_ENABLE || RES_USE_INTERNAL
#define _COMPILE_INTERNAL_RES 1
#else
#define _COMPILE_INTERNAL_RES 0
#endif

#if _COMPILE_INTERNAL_RES
LV_IMG_DECLARE(test_icon);
LV_IMG_DECLARE(wallpaper);
LV_IMG_DECLARE(icon_wifi);
LV_IMG_DECLARE(icon_bright);
LV_IMG_DECLARE(icon_colorwheel);
LV_IMG_DECLARE(icon_Contol);
LV_IMG_DECLARE(default_user);
LV_IMG_DECLARE(img_light);
// extern const lv_font_t my_font_20;

typedef struct {
  res_id_t id;
  uint16_t type;
  const void *src_struct; // 指向初始的 lv_img_dsc_t 或 lv_font_t
} burn_map_t;

// 片上资源表
static const burn_map_t g_burn_table[] = {
    {RES_IMG_TEST, ASSET_TYPE_IMAGE, &test_icon},
    {RES_IMG_WALLPAPER, ASSET_TYPE_IMAGE, &wallpaper},
    {RES_IMG_ICON_WIFI, ASSET_TYPE_IMAGE, &icon_wifi},
    {RES_IMG_ICON_BRIGHT, ASSET_TYPE_IMAGE, &icon_bright},
    {RES_IMG_ICON_COLORWHEEL, ASSET_TYPE_IMAGE, &icon_colorwheel},
    {RES_IMG_ICON_CONTROL, ASSET_TYPE_IMAGE, &icon_Contol},
    {RES_IMG_DEFAULT_USER, ASSET_TYPE_IMAGE, &default_user},
    {RES_IMG_IMG_LIGHT, ASSET_TYPE_IMAGE, &img_light},
};

// 仅烧录模式编译此函数
#if RES_BURN_ENABLE
int res_program_all(void) {
  uint32_t count = sizeof(g_burn_table) / sizeof(g_burn_table[0]);
  if (asset_manager_begin_update(count) != 0)
    return -1;

  for (uint32_t i = 0; i < count; i++) {
    asset_meta_t meta = {0};
    const uint8_t *data = NULL;
    uint32_t size = 0;

    if (g_burn_table[i].type == ASSET_TYPE_IMAGE) {
      lv_img_dsc_t *dsc = (lv_img_dsc_t *)g_burn_table[i].src_struct;
      memcpy(&meta.img_header, &dsc->header, 4);
      data = dsc->data;
      size = dsc->data_size;
    } else if (g_burn_table[i].type == ASSET_TYPE_FONT) {
      lv_font_t *font = (lv_font_t *)g_burn_table[i].src_struct;
      meta.font.line_height = font->line_height;
      meta.font.base_line = font->base_line;
      data = font->dsc; // 字体点阵数据体
      size = 0;         // 字体通常需要根据具体实现计算大小，此处简化为外部已知
    }

    asset_manager_write_res(g_burn_table[i].id, g_burn_table[i].type, meta,
                            data, size);
  }
  return asset_manager_end_update();
}
#endif /* _COMPILE_INTERNAL_RES */

int res_init(void) {
  asset_manager_init();

#if RES_BURN_ENABLE
  log_w("Start to burn resources...");
  if (res_program_all() < 0) {
    log_e("Res Program Failed !!!");
  } else {
    log_i("Res Program Success !!!");
  }
#endif

  // 【修复 1】：必须先初始化影子表，将外部 XIP 地址映射好
  log_i("Mapping external resources...");
  for (uint32_t i = 0; i < RES_COUNT_MAX; i++) {
    uint16_t type; asset_meta_t meta; uint32_t size;
    const void *ptr = asset_manager_get_res_info(i, &type, &meta, &size);

    if (ptr) {
      switch (type) {
        case ASSET_TYPE_IMAGE:
          memcpy(&s_img_dscs[i].header, &meta.img_header, 4);
          s_img_dscs[i].data_size = size;
          s_img_dscs[i].data = ptr;
          break;
        case ASSET_TYPE_FONT:
          s_font_dscs[i].line_height = meta.font.line_height;
          s_font_dscs[i].base_line = meta.font.base_line;
          s_font_dscs[i].dsc = ptr;
          break;
        case ASSET_TYPE_AUDIO:
          s_audio_ptrs[i] = ptr;
          break;
      }
    }
  }

// 【修复 2】：显示测试放在映射完成之后！
#if RES_BURN_ENABLE && RES_DISPLAY_ENABLE
  log_i("Entering Resource Verification Display Loop...");
  lv_obj_t *img_obj = lv_img_create(lv_scr_act());
  lv_obj_center(img_obj);
  uint32_t current_idx = 0;

  while (1) {
    if (g_internal_table[current_idx].type == ASSET_TYPE_IMAGE) {
      // 此时获取的图像，将优先来自外部 Flash (验证烧录结果)
      const lv_img_dsc_t *dsc = res_get_img(g_internal_table[current_idx].id);
      if (dsc) lv_img_set_src(img_obj, dsc);
    }
    current_idx = (current_idx + 1) % INTERNAL_COUNT;
    for (int i = 0; i < 100; i++) { lv_timer_handler(); sys_delay_ms(10); }
  }
#endif

  return 0;
}

const lv_img_dsc_t *res_get_img(res_id_t id) {
#if _LVGL_RES_COMPILE_INTERNAL
  for (uint32_t i = 0; i < sizeof(g_burn_table) / sizeof(g_burn_table[0]);
       i++) {
    if (g_burn_table[i].id == id && g_burn_table[i].type == ASSET_TYPE_IMAGE) {
      return (const lv_img_dsc_t *)g_burn_table[i].src_struct;
    }
  }
#endif
  return (s_img_dscs[id].data) ? &s_img_dscs[id] : NULL;
}

const lv_font_t *res_get_font(res_id_t id) {
#if _LVGL_RES_COMPILE_INTERNAL
  for (uint32_t i = 0; i < sizeof(g_burn_table) / sizeof(g_burn_table[0]);
       i++) {
    if (g_burn_table[i].id == id && g_burn_table[i].type == ASSET_TYPE_FONT) {
      return (const lv_font_t *)g_burn_table[i].src_struct;
    }
  }
#endif
  return (s_font_dscs[id].dsc) ? &s_font_dscs[id] : NULL;
}

#endif /* LVGL_RESOURCE_LVGL_RESOURCE_C_ */
