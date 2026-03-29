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
static bool on_gesture_home(void) {
  log_d("Home gesture triggered at ui.c level");
  app_manager_go_home();
  return false; // 作为基础手势，不强制拦截，除非上方层明确要求拦截
}

// --- Home App Wrapper ---
static lv_obj_t *create_home_wrapper(void) {
  if (!ui_screen_home) {
    ui_screen_home_init(); // 初始化主页
  }
  if (!home_settings.attr.is_loaded) {
    home_settings.attr.is_loaded = 1;
  }
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
  /* 注册其他UI功能 */
  colorwheel_app_register(0);
  device_control_app_register(0);

  // 1. 先绑定主页面基础输入事件 (低优先级，在列表后端)
  input_manager_register_callback(GESTURE_BOTTOM_SWIPE_UP,
                                  on_gesture_home); // 上滑回调,返回home
  input_manager_register_callback(GESTURE_LEFT_SWIPE_IN,
                                  app_manager_pop_screen); // 侧滑回调,返回

  // 2. 初始化系统UI组件 (这些组件通常位于顶层，后注册的回调会在链表前半段，优先触发)
  ui_sys_bar_init();   // 系统顶部状态栏
  ui_sys_panel_init(); // 系统下拉面板 (内部会注册上滑拦截，优先级高于 on_gesture_home)
}

void UI_Start(void) {
  log_i("UI started");
  // 启动主界面
  app_manager_start_app("HOME");
}
