#include "Project_cfg.h"
#if !USE_Simulator
#include "../../System/net_mgr.h"
#else
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    char ssid[33];  // <ssid>
    char mac[18];   // mac地址
    int8_t rssi;    // 信号强度
    uint8_t channel;// 信道号
    uint8_t encryption;      // <ecn>,加密方式
    uint8_t pairwise_cipher; // <pairwise_cipher>,成对加密类型
    uint8_t group_cipher;    // <group_cipher>,组加密类型与pairwise_cipher相同
    uint8_t protocol;        // <wifi_protocol>,支持的协议类型
    uint8_t wps;             // <wps>,是否支持WPS
} wifi_ap_info_t;
#endif
#include "app_manager.h"
#include "elog.h"
#include "ui_wifi_password.h"

#define LOG_TAG "WIFI_SCAN"

static lv_obj_t *wifi_list = NULL;
static lv_obj_t *spinner = NULL;
static lv_obj_t *scan_scr = NULL;

/**
 * @brief wifi列表按钮点击事件回调
 *
 * @param e 事件对象
 */
static void list_btn_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_target(e); // 获取点击的按钮
  const char *ssid =
      lv_list_get_btn_text(wifi_list, btn); // 获取按钮的文本(SSID)
  log_i("Selected SSID: %s", ssid);

  lv_obj_t *pwd_scr = ui_wifi_password_create(ssid); // 创建密码输入界面
  if (pwd_scr) {
    app_manager_push_screen(pwd_scr); // 进入
  }
}

/**
 * @brief 更新扫描结果列表
 *
 * @param results 扫描结果
 * @param count   数量
 */
static void update_scan_list(wifi_ap_info_t *results, uint16_t count) {
  if (spinner) {
    lv_obj_add_flag(spinner, LV_OBJ_FLAG_HIDDEN);
  }

  lv_obj_clean(wifi_list); // 先清空

  if (!results || count == 0) {
    lv_obj_t *label = lv_label_create(wifi_list);
    lv_label_set_text(label, "No networks found or scan failed.");
    return;
  }

  // 遍历结果列表,添加列表按钮
  for (uint16_t i = 0; i < count; i++) {
    lv_obj_t *btn = lv_list_add_btn(wifi_list, LV_SYMBOL_WIFI, results[i].ssid);
    lv_obj_add_event_cb(btn, list_btn_event_cb, LV_EVENT_CLICKED,
                        NULL); // 添加点击事件

    // 显示信号强度
    lv_obj_t *label_rssi = lv_label_create(btn);
    lv_label_set_text_fmt(label_rssi, "%d dBm", results[i].rssi);
    lv_obj_align(label_rssi, LV_ALIGN_RIGHT_MID, -10, 0);
  }
}

/**
 * @brief 扫描完成回调
 *
 * @param arg     用户数据
 * @param ap_list AP列表
 * @param count   数量
 */
static void wifi_scan_completed_cb(void *arg, wifi_ap_list_t *ap_list) {
  if (ap_list == NULL) {
    log_e("Scan failed: ap_list is NULL");
    update_scan_list(NULL, 0);
    return;
  }
  log_i("Scan completed: %d APs found", ap_list->count);
  update_scan_list(ap_list->ap_info, ap_list->count); // 更新结果列表
}

static void start_scan(lv_event_t *e) {
  if (spinner)
    lv_obj_clear_flag(spinner, LV_OBJ_FLAG_HIDDEN); // 显示加载器
  lv_obj_clean(wifi_list);                          // 清空列表

#if !USE_Simulator
  net_mgr_wifi_scan(wifi_scan_completed_cb, NULL); // 调用处理层的api并注册回调
#else
  // 模拟器数据模拟
  static wifi_ap_info_t mock_aps[] = {
      {.ssid = "Simulator_WiFi_1", .rssi = -45},
      {.ssid = "DeepMind_Office", .rssi = -30},
      {.ssid = "Antigravity_Net", .rssi = -20},
      {.ssid = "Public_Free_WiFi", .rssi = -70},
  };
  // 延迟一会模拟真实扫描过程
  lv_timer_t *timer =
      lv_timer_create((lv_timer_cb_t)wifi_scan_completed_cb, 1500, NULL);
  lv_timer_set_repeat_count(timer, 1);
  // 注意：在回调中我们直接传 mock_aps，但回调原型定义了 ap_list 指针。
  // 此处回调会由定时器触发，我们需要修改一下逻辑使其兼容定时器或使用另一个包装
#endif
}

#if USE_Simulator
static void mock_scan_finish_timer_cb(lv_timer_t *t) {
  static wifi_ap_info_t mock_aps[] = {
      {.ssid = "Simulator_WiFi_1", .rssi = -45},
      {.ssid = "DeepMind_Office", .rssi = -30},
      {.ssid = "Antigravity_Net", .rssi = -20},
      {.ssid = "Public_Free_WiFi", .rssi = -70},
  };
  wifi_scan_completed_cb(NULL, mock_aps, 4);
}

// 重新定义模拟器下的 start_scan
static void start_scan_sim(lv_event_t *e) {
  if (spinner)
    lv_obj_clear_flag(spinner, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clean(wifi_list);
  lv_timer_t *timer = lv_timer_create(mock_scan_finish_timer_cb, 1000, NULL);
  lv_timer_set_repeat_count(timer, 1);
}
#endif

/**
 * @brief 创建WiFi扫描界面
 *
 * @return lv_obj_t* 页面对象
 */
lv_obj_t *ui_wifi_scan_create(void) {
  scan_scr = lv_obj_create(NULL);

  lv_obj_t *title = lv_label_create(scan_scr);
  lv_label_set_text(title, "WiFi Networks");
  lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

  // 创建列表
  wifi_list = lv_list_create(scan_scr);
  lv_obj_set_size(wifi_list, LV_PCT(90), LV_PCT(70));
  lv_obj_align(wifi_list, LV_ALIGN_CENTER, 0, 10);

  // 加载器
  spinner = lv_spinner_create(scan_scr, 1000, 60);
  lv_obj_set_size(spinner, 60, 60);
  lv_obj_center(spinner);

  // 刷新按钮
  lv_obj_t *btn_refresh = lv_btn_create(scan_scr);
  lv_obj_set_size(btn_refresh, 120, 50);
  lv_obj_align(btn_refresh, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_obj_t *lbl_refresh = lv_label_create(btn_refresh);
  lv_label_set_text(lbl_refresh, "Scan");
  lv_obj_center(lbl_refresh);
#if !USE_Simulator
  lv_obj_add_event_cb(btn_refresh, start_scan, LV_EVENT_CLICKED, NULL);
  start_scan(NULL); // 开始扫描
#else
  lv_obj_add_event_cb(btn_refresh, start_scan_sim, LV_EVENT_CLICKED, NULL);
  start_scan_sim(NULL);
#endif

  return scan_scr;
}
