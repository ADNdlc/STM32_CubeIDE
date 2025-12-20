#ifndef _UI_HELPERS_H
#define _UI_HELPERS_H

#include <home.h>

#ifdef __cplusplus
extern "C" {
#endif

#define scr_act_width() lv_obj_get_width(lv_scr_act())   // 获取屏幕宽度
#define scr_act_height() lv_obj_get_height(lv_scr_act()) // 获取屏幕高度

/**
 * @brief 屏幕切换
 *
 * @param target 目标屏幕对象指针
 * @param fademode 淡入淡出模式
 * @param spd 切换速度
 * @param delay 延迟时间
 * @param target_init 目标屏幕初始化函数指针
 */
void _ui_screen_change(lv_obj_t **target, lv_scr_load_anim_t fademode, int spd,
                       int delay, void (*target_init)(void));

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
