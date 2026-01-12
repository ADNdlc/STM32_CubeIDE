#include "ui_sys_bar.h"
#include "../../System/sys_state.h"

#include "rtc_hal/rtc_hal.h"
#include "elog.h"

#define LOG_TAG "UI_SYS_BAR"

// 状态栏对象和子组件
static lv_obj_t *bar_obj;
static lv_obj_t *label_time;
static lv_obj_t *label_bat;
static lv_obj_t *label_wifi;

// DEBUG样式
#define BAR_OPA LV_OPA_50

// 时间刷新定时器回调
static void time_timer_cb(lv_timer_t *timer) {
  log_v("time ui cb");
  rtc_time_t rt;
  // 使用RTC获取时间
  if (rtc_hal_get_time(&rt) == 0) {
    log_d("lv time update: %02d:%02d:%02d",rt.hour, rt.minute, rt.second);
    lv_label_set_text_fmt(label_time, "%02d:%02d", rt.hour, rt.minute);
  }
}

// 状态栏更新回调
static void update_ui(const sys_state_t *state) {
  if (state->wifi_connected) {
    lv_label_set_text(label_wifi, LV_SYMBOL_WIFI); // 显示WiFi图标
  } else {
    lv_label_set_text(label_wifi, ""); // 显示空图标
  }

  lv_label_set_text_fmt(label_bat, "%d%%",
                        state->battery_level); // 显示电量百分比
}

void ui_sys_bar_init(void) {
  // 在top layer创建状态栏
  bar_obj = lv_obj_create(lv_layer_top());
  lv_obj_set_size(bar_obj, LV_PCT(100), 30);
  lv_obj_align(bar_obj, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_opa(bar_obj, BAR_OPA, 0);
  lv_obj_set_style_border_width(bar_obj, 0, 0);
  lv_obj_set_style_radius(bar_obj, 0, 0);
  lv_obj_clear_flag(bar_obj, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(bar_obj, LV_OBJ_FLAG_CLICKABLE); // 清除可点击属性

  // Time
  label_time = lv_label_create(bar_obj);
  lv_label_set_text(label_time, "--:--");
  lv_obj_center(label_time);

  // Battery
  label_bat = lv_label_create(bar_obj);
  lv_obj_align(label_bat, LV_ALIGN_RIGHT_MID, -10, 0);

  // WiFi
  label_wifi = lv_label_create(bar_obj);
  lv_obj_align_to(label_wifi, label_bat, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  // 创建时间刷新定时器 (1秒刷新一次)
  lv_timer_create(time_timer_cb, 1000, NULL);

  // 订阅SYS状态更新
  sys_state_subscribe(update_ui);

  // Initial update
  update_ui(sys_state_get());
  time_timer_cb(NULL); // 立即刷新一次时间
}
