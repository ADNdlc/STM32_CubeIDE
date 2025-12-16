#include "ui.h"
#include "System/app_manager.h"
#include "System/input_manager.h"
#include "System/sys_state.h"
#include "SystemUI/ui_sys_bar.h"
#include "SystemUI/ui_sys_panel.h"
#include "screens/ui_screen_home.h"

// --- Bridge Functions ---
static void on_gesture_home(void) { app_manager_go_home(); }

static void on_gesture_pulldown(void) { ui_sys_panel_show(); }

// --- Home App Wrapper ---
static lv_obj_t *create_home_wrapper(void) {
  if (!ui_screen_home) {
      ui_screen_home_init(); // 初始化主屏幕
  }

  return ui_screen_home;
}

static void resume_home_wrapper(struct app_t *app) {
  // Optional: reload specific state
}

static app_def_t home_app_def = {
    .name = "HOME",
    .create = create_home_wrapper,
    .resume = resume_home_wrapper,
    // .destroy is trivial if we rely on LVGL to delete the screen
};

// --- Main Init ---
void ui_init(void) {
  // 初始化核心服务
  sys_state_init();     // 系统状态初始化
  app_manager_init();   // 应用管理器初始化
  input_manager_init(); // 输入管理器初始化

  // 注册app
  app_manager_register(&home_app_def);


  // 初始化系统UI
  ui_sys_bar_init();    // 系统顶部状态栏
  // ui_sys_panel_init();  // 系统下拉面板

  // 绑定输入事件
  input_manager_set_home_gesture_cb(on_gesture_home);   //上滑回调
  input_manager_set_pulldown_cb(on_gesture_pulldown);   //下拉回调

  // 启动主界面
  app_manager_start_app("HOME");
}
