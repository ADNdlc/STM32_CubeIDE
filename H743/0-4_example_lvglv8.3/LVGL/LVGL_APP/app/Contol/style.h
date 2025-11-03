#ifndef __style_H__
#define __style_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "lvgl.h"
#include "util.h"

    void style_init(void);

    void style_deinit(void);

    lv_style_t* style_get_base_default(void);
    lv_style_t* style_get_base_checked(void);
    lv_style_t* style_get_addtab_default(void);
    lv_style_t* style_get_addtab_checked(void);
    lv_style_t* style_get_gradient_style_top(void);
    lv_style_t* style_get_gradient_style_bottom(void);

#ifdef __cplusplus
}
#endif

#endif
