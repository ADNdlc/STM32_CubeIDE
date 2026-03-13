#include "project_cfg.h"
#if !USE_Simulator
#include "System/sys_config.h"
#endif
#include "home.h"
#include "System/app_settings.h"
#include "SystemUI/ui_sys_bar.h"
#include "SystemUI/ui_sys_panel.h"
#include "app_manager.h"
#include "input_manager.h"
#include "screens/ui_screen_home.h"
#include "sys_state.h"

#define LOG_TAG "UI"
#include "elog.h"

static lv_obj_t *create_home_wrapper(void);
static void resume_home_wrapper(struct app_t *app);

static app_def_t home_app_def = {
    .name = "HOME",
    .create = create_home_wrapper,
    .resume = resume_home_wrapper,
    // .destroy is trivial if we rely on LVGL to delete the screen
};

// --- Bridge Functions ---
static void on_gesture_home(void) {
  log_d("Home gesture triggered at ui.c level");
  app_manager_go_home();
}

// --- Home App Wrapper ---
static lv_obj_t *create_home_wrapper(void) {
  if (!ui_screen_home) {
    ui_screen_home_init(); // 初始化主页
  }
#if !USE_Simulator
  if(!sys_config_get()->attr.is_loaded){ // 进入页面如果没有加载，说明注册时未加载成功
    sys_config_set_defaults();  // 使用默认值
    sys_config_get()->attr.is_loaded = 1;
    // 配置在app关闭时自动保存，HOME不会关闭，手动保存
    app_settings_save(home_app_def.name, home_app_def.name);
  }
#endif
  return ui_screen_home;
}

static void resume_home_wrapper(struct app_t *app) {
  // Optional: reload specific state
}



void home_init(void) {
  // 初始化 UI 核心服务
  app_manager_init();   // 应用管理器初始化
  input_manager_init(); // 输入管理器初始化

  sys_config_init();  // 为配置分配空间
  home_app_def.settings = sys_config_get(); // 绑定系统配置
  // 注册home
  app_manager_register(&home_app_def, 0); // 注册时会尝试从flash加载配置文件

  // 初始化系统UI
  ui_sys_bar_init();   // 系统顶部状态栏
  ui_sys_panel_init(); // 系统下拉面板

  // 绑定主页面输入事件
  input_manager_register_callback(GESTURE_BOTTOM_SWIPE_UP,
                                  on_gesture_home); // 上滑回调,返回home
  input_manager_register_callback(GESTURE_LEFT_SWIPE_IN,
                                  app_manager_pop_screen); // 侧滑回调,返回
}

void ui_Start(void) {
  log_i("UI started");
  // 启动主界面
  app_manager_start_app("HOME");
}
