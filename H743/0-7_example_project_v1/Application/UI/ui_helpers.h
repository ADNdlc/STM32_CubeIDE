#ifndef _UI_HELPERS_H
#define _UI_HELPERS_H

#include "ui.h"

#ifdef __cplusplus
extern "C" {
#endif

// Helper function prototypes can go here
void _ui_screen_change(lv_obj_t **target, lv_scr_load_anim_t fademode, int spd,
                       int delay, void (*target_init)(void));

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
