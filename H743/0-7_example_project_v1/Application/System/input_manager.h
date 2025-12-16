#ifndef _INPUT_MANAGER_H
#define _INPUT_MANAGER_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Input Manager
 *
 * 此模块负责处理全局输入事件和手势识别。
 *
 */

// 初始化全局输入/手势系统
void input_manager_init(void);

// 注册全局手势回调
typedef void (*gesture_cb_t)(void);

void input_manager_set_home_gesture_cb(
    gesture_cb_t cb);                                // 设置回到主界面手势回调
void input_manager_set_pulldown_cb(gesture_cb_t cb); // 设置下拉菜单手势回调

#ifdef __cplusplus
}
#endif

#endif
