#include "project_cfg.h"
#if !USE_Simulator
#include "System/sys_config.h"
#endif
#include "System/app_settings.h"
#include "UI/SystemUI/ui_sys_bar.h"
#include "UI/SystemUI/ui_sys_panel.h"
#include "UI/screens/ui_screen_home.h"
#include "app_manager.h"
#include "home.h"
#include "input_manager.h"
#include "sys_state.h"

#include "Colorwheel/colorwheel.h"
#include "device_control/device_control.h"

#define LOG_TAG "UI"
#include "elog.h"

static lv_obj_t *create_home_wrapper(void);
static void resume_home_wrapper(struct app_t *app);

typedef enum {
    HOME_CFG_WALLPAPER,
    HOME_CFG_KEY_MAX
} home_config_key_t;

static app_config_t home_configs[HOME_CFG_KEY_MAX];
static app_settings_t home_settings = {
    .configs = home_configs,
    .count = HOME_CFG_KEY_MAX,
    .hash = 0x5678,
};

static app_def_t home_app_def = {
    .name = "HOME",
    .create = create_home_wrapper,
    .resume = resume_home_wrapper,
    .settings = &home_settings,
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
  if (!home_settings.attr.is_loaded) { // 进入页面如果没有加载，说明注册时未加载成功
    // 设置 HOME 的默认值 (目前为空或预留)
    home_settings.attr.is_loaded = 1;
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

  sys_config_init();                        // 为系统配置分配空间和加载
  // home_app_def.settings 已在定义时静态绑定到 &home_settings
  // 注册home
  app_manager_register(&home_app_def, 0); // 注册时会尝试从文件系统加载 HOME.json
  colorwheel_app_register(0);
  device_control_app_register(0);

  // 初始化系统UI
  ui_sys_bar_init();   // 系统顶部状态栏
  ui_sys_panel_init(); // 系统下拉面板

  // 绑定主页面输入事件
  input_manager_register_callback(GESTURE_BOTTOM_SWIPE_UP,
                                  on_gesture_home); // 上滑回调,返回home
  input_manager_register_callback(GESTURE_LEFT_SWIPE_IN,
                                  app_manager_pop_screen); // 侧滑回调,返回
}

void UI_Start(void) {
  log_i("UI started");
  // 启动主界面
  app_manager_start_app("HOME");
}
