
#ifndef _UI_SCREEN_HOME_H
#define _UI_SCREEN_HOME_H

#include <home.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Home UI
 *
 * 此模块负责创建和管理主屏幕UI，Home提供了其他功能的UI的入口。
 *
 */
#define DEBUG 1

extern lv_obj_t *ui_screen_home; // Home屏幕对象

void ui_screen_home_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
