#ifndef _UI_H
#define _UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#include "components/ui_comp_app_icon.h"
#include "screens/ui_screen_home.h"
#include "ui_events.h"


// Global UI Init
void ui_init(void);

// Resources
LV_IMG_DECLARE(icon_home);
LV_IMG_DECLARE(wallpaper);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
