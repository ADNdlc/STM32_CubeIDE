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
  GESTURE_LEFT_SWIPE_IN,   // 左侧边缘向内滑 (返回)
  GESTURE_TYPE_MAX
} gesture_type_t;

// 手势回调函数类型 (返回 true 表示消费掉该手势，不再向下传递)
typedef bool (*gesture_cb_t)(void);

/**
 * @brief Input Manager
 *
 * 此模块负责处理全局输入事件和手势识别。
 *
 */

// 初始化全局输入/手势系统
void input_manager_init(void);

/**
 * @brief 注册全局手势回调(后注册的优先级更高)
 *
 * @param type 手势类型
 * @param cb 回调函数
 */
void input_manager_register_callback(gesture_type_t type, gesture_cb_t cb);

/**
 * @brief 处理用户活跃信号 (用于刷新屏幕超时)
 */
void input_manager_handle_activity(void);

#ifdef __cplusplus
}
#endif

#endif
