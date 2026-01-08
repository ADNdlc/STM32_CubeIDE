/*
 * lv_util.h
 *
 *  Created on: Jan 7, 2026
 *      Author: 12114
 */

#ifndef LV_UTIL_LV_UTIL_H_
#define LV_UTIL_LV_UTIL_H_

#include "lvgl.h"

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

/**
 * @brief 创建自定义样式滑块
 * @param color  颜色
 * @param Screens
 * @return
 */
lv_obj_t* simple_slider_create(lv_color_t color, lv_obj_t* parent);

/**
 * @brief 这里是DEBUG用的测试函数
 *
 */
#ifndef NODEBUG

void test_layout_grid(lv_obj_t* grid_obj, uint8_t row_num, uint8_t col_num);


#endif // NODEBUG

#endif /* LV_UTIL_LV_UTIL_H_ */
