#ifndef _UI_SCREEN_COLORWHEEL_H
#define _UI_SCREEN_COLORWHEEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

// Screen initialization
void ui_screen_colorwheel_init(void);

// Register the app with the app manager
void colorwheel_app_register(int page_index);

// Screen object
extern lv_obj_t * ui_screen_colorwheel;

#ifdef __cplusplus
}
#endif

#endif