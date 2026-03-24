#ifndef __style_H__
#define __style_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "lvgl.h"

    void style_init(void);

    void style_deinit(void);

    lv_style_t* style_get_base_default(void);
    lv_style_t* style_get_base_checked(void);
    lv_style_t* style_get_addtab_default(void);
    lv_style_t* style_get_addtab_checked(void);
    lv_style_t* style_get_gradient_style_top(void);
    lv_style_t* style_get_gradient_style_bottom(void);

    void style_tabview_simple(lv_obj_t* tabview, lv_style_t* _default, lv_style_t* _checked);
    void obj_align_card(lv_obj_t* obj, uint8_t count, lv_coord_t width, lv_coord_t height);

#ifdef __cplusplus
}
#endif

#endif
