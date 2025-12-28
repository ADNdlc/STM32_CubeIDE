#ifndef _UI_H
#define _UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#include "components/ui_comp_app_icon.h"
#include "screens/ui_screen_home.h"
#include "ui_events.h"

void home_init(void);   // ui功能初始化(初始化后注册)
void ui_Start(void);    // ui启动

// 资源声明
LV_IMG_DECLARE(icon_home);
//LV_IMG_DECLARE(wallpaper);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
