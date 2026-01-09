#include "SystemUI/ui_sys_bar.h"
#include "SystemUI/ui_sys_panel.h"
#include "app_manager.h"
#include "input_manager.h"
#include "screens/ui_screen_home.h"
#include "sys_state.h"
#include "home.h"
#define LOG_TAG "UI"
#include "elog.h"

// --- Bridge Functions ---
static void on_gesture_home(void) {
  log_d("UI: Home gesture triggered at ui.c level");
  app_manager_go_home();
}

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

void home_init(void) {
  // 初始化 UI 核心服务
  app_manager_init();   // 应用管理器初始化
  input_manager_init(); // 输入管理器初始化

  // 注册home
  app_manager_register(&home_app_def, 0);

  // 初始化系统UI
  ui_sys_bar_init();   // 系统顶部状态栏
  ui_sys_panel_init(); // 系统下拉面板

  // 绑定主页面输入事件
  input_manager_register_callback(GESTURE_BOTTOM_SWIPE_UP,
                                  on_gesture_home); // 上滑回调,返回home
}

void ui_Start(void) {
  log_i("UI started");
  // 启动主界面
  app_manager_start_app("HOME");
}
