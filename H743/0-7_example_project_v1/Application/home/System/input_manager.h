#ifndef _INPUT_MANAGER_H
#define _INPUT_MANAGER_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// 手势类型
typedef enum {
  GESTURE_TOP_PRESS,       // 顶部区域按下
  GESTURE_BOTTOM_PRESS,    // 底部区域按下
  GESTURE_TOP_SWIPE_DOWN,  // 顶部区域下滑
  GESTURE_BOTTOM_SWIPE_UP, // 底部区域上滑
  GESTURE_TYPE_MAX
} gesture_type_t;

// 手势回调函数类型
typedef void (*gesture_cb_t)(void);

/**
 * @brief Input Manager
 *
 * 此模块负责处理全局输入事件和手势识别。
 *
*/

// 初始化全局输入/手势系统
void input_manager_init(void);

/**
 * @brief 注册全局手势回调
 *
 * @param type 手势类型
 * @param cb 回调函数
 */
void input_manager_register_callback(gesture_type_t type, gesture_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif
