#include "input_manager.h"
#include <stdio.h>

#define EDGE_HEIGHT 30       // 手势区域高度
#define GESTURE_THRESHOLD 50 // 触发手势的最小位移

static lv_obj_t *top_zone;
static lv_obj_t *bottom_zone;

static gesture_cb_t cb_home = NULL;
static gesture_cb_t cb_pulldown = NULL;

static lv_point_t start_pos;     // 手势开始位置
static bool is_tracking = false; // 是否正在跟踪手势()

/**
 * @brief 全局手势事件回调
 *
 * @param e 触发Event的对象
 */
static void gesture_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e); // 事件代码
  lv_obj_t *target = lv_event_get_target(e);   // 最初触发Event的对象
  lv_indev_t *indev = lv_indev_get_act(); // 当前正在处理的输入设备(display)

  if (!indev)
    return;

  if (code == LV_EVENT_PRESSED) {          // 如果是按下事件
    lv_indev_get_point(indev, &start_pos); // 记录起始位置,并开始跟踪
    is_tracking = true;
  } else if (code == LV_EVENT_PRESSING &&
             is_tracking) { // 持续按压且 正在跟踪手势
    lv_point_t current;
    lv_indev_get_point(indev, &current);
    lv_coord_t dy = current.y - start_pos.y;

    // 顶部下拉(无需松开)
    if (target == top_zone &&
        dy > GESTURE_THRESHOLD) { // 最初触发手势的是top_zone ,且dy超过阈值
      if (cb_pulldown) {
        cb_pulldown();       // 触发回调(下拉菜单)
        is_tracking = false; // 停止跟踪
        // lv_indev_wait_release(indev); // 等待释放(防止误触)
        lv_indev_reset(indev, NULL);
      }
    }
  } else if (code == LV_EVENT_RELEASED && is_tracking) { // 释放手势
    lv_point_t end;
    lv_indev_get_point(indev, &end);
    lv_coord_t dy = end.y - start_pos.y;

    // 底部上滑
    if (target == bottom_zone &&
        dy < -GESTURE_THRESHOLD) { // 最初触发手势的是bottom_zone ,且dy超过阈值
      if (cb_home) {
        cb_home(); // 触发回调(回到主界面)
      }
    }
    is_tracking = false;
  }
}

/**
 * @brief 创建一个手势区
 *
 */
  lv_obj_t *create_gesture_zone(lv_obj_t *parent, lv_coord_t w, lv_coord_t h) {
  lv_obj_t *zone = lv_obj_create(parent);
  lv_obj_set_size(zone, w, h);
  lv_obj_set_style_bg_opa(zone, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(zone, 0, 0);
  lv_obj_add_event_cb(zone, gesture_event_cb, LV_EVENT_ALL,
                      NULL); // 绑定统一手势处理回调
  lv_obj_clear_flag(zone, LV_OBJ_FLAG_SCROLLABLE);
  return zone;
}

/**
 * @brief 创建并初始化所有手势区
 *
 */
void input_manager_init(void) {
  // 手势区创建在SYS层
  lv_obj_t *parent = lv_layer_sys();

  // 顶部手势区
  top_zone = create_gesture_zone(parent, LV_PCT(100), EDGE_HEIGHT);
  // 底部手势区
  bottom_zone = create_gesture_zone(parent, LV_PCT(100), EDGE_HEIGHT);
}

/* 外部使用的手势回调设置 */
void input_manager_set_home_gesture_cb(gesture_cb_t cb) { cb_home = cb; }

void input_manager_set_pulldown_cb(gesture_cb_t cb) { cb_pulldown = cb; }
