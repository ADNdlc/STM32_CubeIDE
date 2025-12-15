#include "input_manager.h"
#include <stdio.h>

#define EDGE_HEIGHT 30
#define GESTURE_THRESHOLD 50

static lv_obj_t *top_zone;
static lv_obj_t *bottom_zone;

static gesture_cb_t cb_home = NULL;
static gesture_cb_t cb_pulldown = NULL;

static lv_point_t start_pos;
static bool is_tracking = false;

static void gesture_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  lv_indev_t *indev = lv_indev_get_act();

  if (!indev)
    return;

  if (code == LV_EVENT_PRESSED) {
    lv_indev_get_point(indev, &start_pos);
    is_tracking = true;
  } else if (code == LV_EVENT_PRESSING && is_tracking) {
    lv_point_t current;
    lv_indev_get_point(indev, &current);
    lv_coord_t dy = current.y - start_pos.y;

    // Top Pull Down
    if (target == top_zone && dy > GESTURE_THRESHOLD) {
      if (cb_pulldown) {
        cb_pulldown();
        is_tracking = false;
        lv_indev_wait_release(indev); // Reset input
      }
    }
  } else if (code == LV_EVENT_RELEASED && is_tracking) {
    lv_point_t end;
    lv_indev_get_point(indev, &end);
    lv_coord_t dy = end.y - start_pos.y;

    // Bottom Swipe Up
    if (target == bottom_zone && dy < -GESTURE_THRESHOLD) {
      if (cb_home) {
        cb_home();
      }
    }
    is_tracking = false;
  }
}

void input_manager_init(void) {
  // Create zones on sys layer (topmost)
  lv_obj_t *parent = lv_layer_sys();

  top_zone = lv_obj_create(parent);
  lv_obj_set_size(top_zone, LV_PCT(100), EDGE_HEIGHT);
  lv_obj_align(top_zone, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_opa(top_zone, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(top_zone, 0, 0);
  lv_obj_add_event_cb(top_zone, gesture_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_clear_flag(top_zone, LV_OBJ_FLAG_SCROLLABLE);

  bottom_zone = lv_obj_create(parent);
  lv_obj_set_size(bottom_zone, LV_PCT(100), EDGE_HEIGHT);
  lv_obj_align(bottom_zone, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_bg_opa(bottom_zone, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(bottom_zone, 0, 0);
  lv_obj_add_event_cb(bottom_zone, gesture_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_clear_flag(bottom_zone, LV_OBJ_FLAG_SCROLLABLE);
}

void input_manager_set_home_gesture_cb(gesture_cb_t cb) { cb_home = cb; }

void input_manager_set_pulldown_cb(gesture_cb_t cb) { cb_pulldown = cb; }
