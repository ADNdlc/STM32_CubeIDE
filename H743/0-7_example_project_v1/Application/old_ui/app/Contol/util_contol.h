#ifndef __util_contol_H__
#define __util_contol_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "lvgl.h"
#include "util.h"
#include <assert.h>

void style_tabview_simple(lv_obj_t* tabview, lv_style_t* _default, lv_style_t* _checked);
void obj_align_card(lv_obj_t* obj, uint8_t count, lv_coord_t width, lv_coord_t height);

#ifdef __cplusplus
}
#endif

#endif
