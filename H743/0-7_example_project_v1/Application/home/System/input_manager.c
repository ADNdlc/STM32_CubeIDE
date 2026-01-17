#include "input_manager.h"
#include <stdlib.h>

#define LOG_TAG "INPUT_MGR"
#include "elog.h"

#define EDGE_HEIGHT 30       // 手势区域高度
#define GESTURE_THRESHOLD 30 // 触发手势的最小位移

// 链表节点定义
typedef struct gesture_node {
  gesture_cb_t cb;
  struct gesture_node *next;
} gesture_node_t;

// 手势回调链表头数组
static gesture_node_t *gesture_heads[GESTURE_TYPE_MAX] = {0};

static lv_obj_t *top_zone;
static lv_obj_t *bottom_zone;
static lv_obj_t *left_zone;

static lv_point_t start_pos;     // 手势开始位置
static bool is_tracking = false; // 是否正在跟踪手势

// ... (fire_gesture remains same)

static void gesture_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e); // 事件代码
  lv_obj_t *target = lv_event_get_target(e);   // 最初触发Event的对象
  lv_indev_t *indev = lv_indev_get_act(); // 当前正在处理的输入设备(display)
  if (!indev)
    return;

  /* 按下事件 */
  if (code == LV_EVENT_PRESSED) {
    lv_indev_get_point(indev, &start_pos); // 记录起始位置,并开始跟踪
    is_tracking = true;
    if (target == top_zone) {
      fire_gesture(GESTURE_TOP_PRESS); // 顶部区域按下事件
    } else if (target == bottom_zone) {
      fire_gesture(GESTURE_BOTTOM_PRESS); // 底部区域按下事件
    }
  }
  /* 持续按压且 正在跟踪手势 */
  else if (code == LV_EVENT_PRESSING && is_tracking) {
    lv_point_t current;
    lv_indev_get_point(indev, &current);
    lv_coord_t dy = current.y - start_pos.y;
    lv_coord_t dx = current.x - start_pos.x;

    if (target == top_zone &&
        dy > GESTURE_THRESHOLD) { // 最初触发手势的是top_zone ,且dy超过阈值
      log_d("InputManager: Top swipe down detected (dy=%d)", dy);
      fire_gesture(GESTURE_TOP_SWIPE_DOWN); // 顶部下拉(无需松开)
      is_tracking = false;                  // 停止跟踪，避免重复触发
      lv_indev_reset(indev, NULL);          // 移交当前输入给panel_bg(没有松开)
    } else if (target == left_zone && dx > GESTURE_THRESHOLD) {
      log_d("InputManager: Left swipe in detected (dx=%d)", dx);
      fire_gesture(GESTURE_LEFT_SWIPE_IN); // 侧边右滑
      is_tracking = false;
      lv_indev_reset(indev, NULL);
    }
  }
  /* 释放手势 */
  else if ((code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) &&
           is_tracking) {
    lv_point_t end;
    lv_indev_get_point(indev, &end);
    lv_coord_t dy = end.y - start_pos.y;

    if (target == bottom_zone &&
        dy < -GESTURE_THRESHOLD) { // 最初触发手势的是bottom_zone ,且dy超过阈值
      log_d("InputManager: Bottom swipe up detected (dy=%d)", dy);
      fire_gesture(GESTURE_BOTTOM_SWIPE_UP); // 底部上滑(需要松开)
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
  lv_obj_align(top_zone, LV_ALIGN_TOP_MID, 0, 0);

  // 底部手势区
  bottom_zone = create_gesture_zone(parent, LV_PCT(100), EDGE_HEIGHT);
  lv_obj_align(bottom_zone, LV_ALIGN_BOTTOM_MID, 0, 0);

  // 左侧手势区
  left_zone = create_gesture_zone(parent, 20, LV_PCT(100)); // 20px wide
  lv_obj_align(left_zone, LV_ALIGN_LEFT_MID, 0, 0);
}

void input_manager_register_callback(gesture_type_t type, gesture_cb_t cb) {
  if (type >= GESTURE_TYPE_MAX || !cb)
    return;

  // 分配新节点 (使用lvgl内存管理或系统malloc)
  gesture_node_t *node = (gesture_node_t *)lv_mem_alloc(sizeof(gesture_node_t));
  if (!node)
    return;

  node->cb = cb;
  // 头插法
  node->next = gesture_heads[type];
  gesture_heads[type] = node;
}
