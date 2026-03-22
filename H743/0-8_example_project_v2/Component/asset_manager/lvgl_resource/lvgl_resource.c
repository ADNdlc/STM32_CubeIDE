/*
 * lvgl_resource.c
 *
 *  Created on: Mar 17, 2026
 *      Author: 12114
 */

#ifndef LVGL_RESOURCE_LVGL_RESOURCE_C_
#define LVGL_RESOURCE_LVGL_RESOURCE_C_

#include "Project_cfg.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "lvgl_resource.h"
#include "asset_manager.h"
#include "elog.h"
#define LOG_TAG "LV_RES"

#if RES_DISPLAY_ENABLE

#endif

// 各种类型的影子描述符表
static lv_img_dsc_t s_img_dscs[RES_COUNT_MAX];
static lv_font_t s_font_dscs[RES_COUNT_MAX];
static const void *s_audio_ptrs[RES_COUNT_MAX];

/* --------- 仅烧录模式编译图片数据 --------- */
#if RES_BURN_MODE
// lvgl资源声明
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

};

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

#endif /* RES_BURN_MODE */

int res_init(void) {
  asset_manager_init();
#if RES_BURN_MODE
  log_w("Start to burn resources");
  if (res_program_all() < 0) {
    log_e("Res Program Failed !!!");
  }
#endif
  log_i("Start to init resources");
  for (uint32_t i = 0; i < RES_COUNT_MAX; i++) {
    uint16_t type;
    asset_meta_t meta;
    uint32_t size;
    const void *ptr = asset_manager_get_res_info(i, &type, &meta, &size);

    if (!ptr) {
      log_e("Resource %d not found", i);
      continue;
    }

    switch (type) {
    case ASSET_TYPE_IMAGE:
      memcpy(&s_img_dscs[i].header, &meta.img_header, 4);
      s_img_dscs[i].data_size = size;
      s_img_dscs[i].data = ptr;
      break;

    case ASSET_TYPE_FONT:
      s_font_dscs[i].line_height = meta.font.line_height;
      s_font_dscs[i].base_line = meta.font.base_line;
      s_font_dscs[i].dsc = ptr; // 直接指向 XIP 地址
      // 注意：字体的 get_glyph_dsc 等函数指针需要在此手动赋值或指向通用解析函数
      break;

    case ASSET_TYPE_AUDIO:
      s_audio_ptrs[i] = ptr;
      break;
    }
  }
  return 0;
}

const lv_img_dsc_t *res_get_img(res_id_t id) {
  return (s_img_dscs[id].data) ? &s_img_dscs[id] : NULL;
}

const lv_font_t *res_get_font(res_id_t id) {
  return (s_font_dscs[id].dsc) ? &s_font_dscs[id] : NULL;
}

#endif /* LVGL_RESOURCE_LVGL_RESOURCE_C_ */
