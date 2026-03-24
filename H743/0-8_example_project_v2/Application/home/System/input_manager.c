#include "input_manager.h"
#include <stdlib.h>

#define LOG_TAG "INPUT_MGR"
#include "elog.h"

#define EDGE_HEIGHT 30       // 手势区域高度 (由于不遮挡UI，可适当调宽提高灵敏度)
#define GESTURE_THRESHOLD 35 // 触发手势的最小位移

typedef struct gesture_node {
  gesture_cb_t cb;
  struct gesture_node *next;
} gesture_node_t;

static gesture_node_t *gesture_heads[GESTURE_TYPE_MAX] = {0};

static lv_point_t start_pos;
static bool is_tracking = false;
static bool gesture_intercepted = false; // 是否已经拦截了该次触摸

// 保存底层的原始触摸读取回调
static void (*original_read_cb)(lv_indev_drv_t * drv, lv_indev_data_t * data) = NULL;

static void fire_gesture(gesture_type_t type) {
  if (type >= GESTURE_TYPE_MAX) return;
  gesture_node_t *node = gesture_heads[type];
  while (node) {
    if (node->cb) node->cb();
    node = node->next;
  }
}

/**
 * @brief 自定义输入读取拦截器 (Hook)
 * 在硬件驱动填入数据后，LVGL核心处理数据前，进行手势拦截
 */
static void custom_indev_read_cb(lv_indev_drv_t * drv, lv_indev_data_t * data) {
    // 1. 先调用原始的驱动读取函数，获取真实的物理坐标(data->point)和状态(data->state)
    if (original_read_cb) {
        original_read_cb(drv, data);
    }

    static lv_indev_state_t last_state = LV_INDEV_STATE_RELEASED;
    lv_point_t current = data->point;
    lv_indev_state_t state = data->state;
    lv_coord_t screen_h = lv_disp_get_ver_res(drv->disp);

    /* --- 1. 刚按下瞬间 --- */
    if (state == LV_INDEV_STATE_PRESSED && last_state == LV_INDEV_STATE_RELEASED) {
        start_pos = current;
        is_tracking = true;
        gesture_intercepted = false; // 复位拦截标志

        if (start_pos.y < EDGE_HEIGHT) {
            fire_gesture(GESTURE_TOP_PRESS);
        } else if (start_pos.y > (screen_h - EDGE_HEIGHT)) {
            fire_gesture(GESTURE_BOTTOM_PRESS);
        }
    }
    /* --- 2. 持续按压滑动中 --- */
    else if (state == LV_INDEV_STATE_PRESSED && is_tracking) {
        lv_coord_t dy = current.y - start_pos.y;
        lv_coord_t dx = current.x - start_pos.x;

        // 顶部下拉
        if (start_pos.y < EDGE_HEIGHT && dy > GESTURE_THRESHOLD) {
            log_d("Top swipe down");
            fire_gesture(GESTURE_TOP_SWIPE_DOWN);
            is_tracking = false;
            gesture_intercepted = true; // 触发手势后，开启物理拦截！
        }
        // 左侧右滑
        else if (start_pos.x < 25 && dx > GESTURE_THRESHOLD) {
            log_d("Left swipe in");
            fire_gesture(GESTURE_LEFT_SWIPE_IN);
            is_tracking = false;
            gesture_intercepted = true;
        }
    }
    /* --- 3. 释放瞬间 --- */
    else if (state == LV_INDEV_STATE_RELEASED && last_state == LV_INDEV_STATE_PRESSED) {
        if (is_tracking) {
            lv_coord_t dy = current.y - start_pos.y;
            // 底部上滑 (必须等手指松开时才触发)
            if (start_pos.y > (screen_h - EDGE_HEIGHT) && dy < -GESTURE_THRESHOLD) {
                log_d("Bottom swipe up");
                fire_gesture(GESTURE_BOTTOM_SWIPE_UP);
            }
            is_tracking = false;
        }
        gesture_intercepted = false; // 手指松开，解除拦截
    }

    last_state = state;
}

void input_manager_init(void) {
    // 找到系统的触摸屏输入设备
    lv_indev_t * indev = lv_indev_get_next(NULL);

    while(indev) {
        if(lv_indev_get_type(indev) == LV_INDEV_TYPE_POINTER) {
            // 获取底层驱动结构体
            lv_indev_drv_t * drv = indev->driver;

            // 备份原来的硬件读取回调
            if (drv->read_cb != custom_indev_read_cb) {
                original_read_cb = drv->read_cb;

                // 将读取回调替换为“劫持层”
                drv->read_cb = custom_indev_read_cb;
            }
            break; // 假设只挂载第一个触摸屏
        }
        indev = lv_indev_get_next(indev);
    }

    log_i("Input manager initialized (Hook Driver Mode).");
}

void input_manager_register_callback(gesture_type_t type, gesture_cb_t cb) {
  if (type >= GESTURE_TYPE_MAX || !cb) return;

  gesture_node_t *node = (gesture_node_t *)lv_mem_alloc(sizeof(gesture_node_t));
  if (!node) return;

  node->cb = cb;
  node->next = gesture_heads[type];
  gesture_heads[type] = node;
}
