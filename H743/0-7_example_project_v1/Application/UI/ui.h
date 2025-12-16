#ifndef _UI_H
#define _UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#include "components/ui_comp_app_icon.h"
#include "screens/ui_screen_home.h"
#include "ui_events.h"

// 全局UI入口
void ui_init(void);

// 资源声明
LV_IMG_DECLARE(icon_home);
LV_IMG_DECLARE(wallpaper);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
