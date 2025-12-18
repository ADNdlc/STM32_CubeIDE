#ifndef __util_H__
#define __util_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "../lvgl.h"
#include <assert.h>

#define scr_act_width() lv_obj_get_width(lv_scr_act())   // 获取屏幕宽度
#define scr_act_height() lv_obj_get_height(lv_scr_act()) // 获取屏幕高度

lv_obj_t* slider_create(lv_color_t color, lv_obj_t* parent);
void anim_scroll_disable_cb(lv_event_t* e);
void _test_layout_grid_(lv_obj_t* grid_obj, uint8_t row, uint8_t col);

#ifdef __cplusplus
}
#endif

#endif
