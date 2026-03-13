#include "ui_wifi_password.h"
#include "../../System/net_mgr.h"
#include "app_manager.h"
#include "elog.h"
#include "project_cfg.h"
#include <string.h>


#define LOG_TAG "WIFI_PWD"

static char target_ssid[33];
static lv_obj_t *textarea = NULL;

static void connect_btn_event_cb(lv_event_t *e) {
  const char *pwd = lv_textarea_get_text(textarea);
  log_i("Connecting to %s with password...", target_ssid);

#if !USE_Simulator
  net_mgr_wifi_connect_manual(target_ssid, pwd);
#else
  log_i("[Simulator] Manual connect SSID: %s, PWD: %s", target_ssid, pwd);
#endif

  // 返回到之前的页面
  // 连续弹出两次以回到系统面板打开前的状态 (Scan -> Password)
  // 注意：app_manager_pop_screen 会删除当前对象
  app_manager_pop_screen();
  app_manager_pop_screen();
}

static void cancel_btn_event_cb(lv_event_t *e) { app_manager_pop_screen(); }

static void eye_btn_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_target(e);
  lv_obj_t *label = lv_obj_get_child(btn, 0);
  bool pwd_mode = lv_textarea_get_password_mode(textarea);

  if (pwd_mode) {
    lv_textarea_set_password_mode(textarea, false);
    lv_label_set_text(label, LV_SYMBOL_EYE_OPEN);
  } else {
    lv_textarea_set_password_mode(textarea, true);
    lv_label_set_text(label, LV_SYMBOL_EYE_CLOSE);
  }
}

/**
 * @brief 创建 WiFi 密码输入界面
 *
 * @param ssid 传入要连接wifi的ssid
 * @return lv_obj_t* 页面
 */
lv_obj_t *ui_wifi_password_create(const char *ssid) {
  if (ssid) {
    strncpy(target_ssid, ssid, sizeof(target_ssid) - 1);
    target_ssid[sizeof(target_ssid) - 1] = '\0';
  } else {
    target_ssid[0] = '\0';
  }

  lv_obj_t *scr = lv_obj_create(NULL); // LVGL根对象

  lv_obj_t *label = lv_label_create(scr);
  lv_label_set_text_fmt(label, "Connect to: %s", target_ssid);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);

  // 密码输入框
  textarea = lv_textarea_create(scr);
  lv_textarea_set_password_mode(textarea, true);
  lv_textarea_set_password_show_time(textarea, 500);
  lv_textarea_set_one_line(textarea, true);               // 单行模式
  lv_textarea_set_placeholder_text(textarea, "Password"); // 占位提示
  lv_obj_set_width(textarea, LV_PCT(80));
  lv_obj_align(textarea, LV_ALIGN_TOP_MID, 0, 60);

  // 增加显示/隐藏密码按钮
  lv_obj_t *disply_mode_btn = lv_btn_create(scr);
  lv_obj_set_size(disply_mode_btn, 40, 40);
  // 无颜色无边框
  lv_obj_set_style_bg_opa(disply_mode_btn, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(disply_mode_btn, 0, 0);
  lv_obj_set_style_shadow_width(disply_mode_btn, 0, 0);
  lv_obj_set_style_text_color(disply_mode_btn, lv_palette_main(LV_PALETTE_GREY),
                              0);

  lv_obj_t *eye_label = lv_label_create(disply_mode_btn);
  lv_label_set_text(eye_label, LV_SYMBOL_EYE_OPEN);
  lv_obj_center(eye_label);

  // 放在输入框右侧内部（对齐到textarea的右侧中间）
  lv_obj_align_to(disply_mode_btn, textarea, LV_ALIGN_RIGHT_MID, 0, 0);
  lv_obj_add_event_cb(disply_mode_btn, eye_btn_event_cb, LV_EVENT_CLICKED,
                      NULL);

  lv_obj_t *kb = lv_keyboard_create(scr);
  lv_keyboard_set_textarea(kb, textarea);
  // 较小的键盘或特定模式
  lv_obj_set_size(kb, LV_PCT(100), LV_PCT(45));
  lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, -55);

  lv_obj_t *btn_connect = lv_btn_create(scr);
  lv_obj_set_size(btn_connect, 120, 45);
  lv_obj_align(btn_connect, LV_ALIGN_BOTTOM_RIGHT, -20, -5);
  lv_obj_t *lbl_connect = lv_label_create(btn_connect);
  lv_label_set_text(lbl_connect, "Connect");
  lv_obj_center(lbl_connect);
  lv_obj_add_event_cb(btn_connect, connect_btn_event_cb, LV_EVENT_CLICKED,
                      NULL);

  lv_obj_t *btn_cancel = lv_btn_create(scr);
  lv_obj_set_size(btn_cancel, 120, 45);
  lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_LEFT, 20, -5);
  lv_obj_t *lbl_cancel = lv_label_create(btn_cancel);
  lv_label_set_text(lbl_cancel, "Cancel");
  lv_obj_center(lbl_cancel);
  lv_obj_add_event_cb(btn_cancel, cancel_btn_event_cb, LV_EVENT_CLICKED, NULL);

  return scr;
}
