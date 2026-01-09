#ifndef _UI_SYS_PANEL_H
#define _UI_SYS_PANEL_H

#include "lvgl.h"
#include "project_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 系统面板初始化(下拉菜单)
 *
 */

void ui_sys_panel_init(void);
void ui_sys_panel_show(void);
void ui_sys_panel_hide(void);
bool ui_sys_panel_is_visible(void);

#ifdef __cplusplus
}
#endif

#endif
