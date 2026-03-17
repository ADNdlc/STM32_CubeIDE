/*
 * lvgl_resource.c
 *
 *  Created on: Mar 17, 2026
 *      Author: 12114
 */

#ifndef LVGL_RESOURCE_LVGL_RESOURCE_C_
#define LVGL_RESOURCE_LVGL_RESOURCE_C_

#include "lvgl_resource.h"
#include "elog.h"

// LVGL资源声明
#include "lvgl.h"
LV_IMG_DECLARE(test_icon);

// 资源表项
typedef struct {
  lvgl_res_id_t id;
  const void *src;
} res_map_t;

// 资源表
static const res_map_t res_table[RES_IMG_COUNT_MAX] = {
    // 直接引用(放在片上)
    {RES_IMG_TEST, &test_icon},
};

const void *res_get_src(lvgl_res_id_t id) {
  if (id >= RES_IMG_COUNT_MAX) {
    return NULL;
  }

  for (size_t i = 0; i < sizeof(res_table) / sizeof(res_table[0]); i++) {
    if (res_table[i].id == id) {
      return res_table[i].src;
    }
  }
  return NULL;
}

#endif /* LVGL_RESOURCE_LVGL_RESOURCE_C_ */
