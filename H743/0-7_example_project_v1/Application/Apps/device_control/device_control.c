#include "device_control.h"
#include "System/Contol_controller.h"
#include "UI/components/style_util.h"
#include "UI/screens/Contol_view.h"
#include "app_manager.h"
#include "core_app.h"
#include "elog.h"
#include "lv_util.h"
#include "res_manager.h"
#include <stddef.h>

#define LOG_TAG "DevControl"

// 接口函数
static lv_obj_t *create_device_control_screen(void);
static void destroy_device_control_screen(struct app_t *app);
static void pause_device_control_screen(struct app_t *app);
static void resume_device_control_screen(struct app_t *app);

static app_settings_t dvice_control_settings; // 存储配置
app_settings_t *get_self_settings(void) { return &dvice_control_settings; }

// 定义 Device Control 应用
static app_def_t dvice_control_app_def = {
    .name = "DevControl",
    .icon = NULL, // Set during registration
    .settings = &dvice_control_settings,
    .create = create_device_control_screen,
    .destroy = destroy_device_control_screen,
    .pause = pause_device_control_screen,
    .resume = resume_device_control_screen,
};

/**
 * @brief 注册 dvice_control 应用
 * @param page_index 放置页码
 */
void device_control_app_register(int page_index) {
  log_i("Registering Device Control App at page %d", page_index);
  dvice_control_app_def.icon = res_get_src(RES_IMG_ICON_CONTROL);
  app_manager_register(&dvice_control_app_def, page_index);
}

/**
 * @brief 创建 dvice_control 屏幕
 */
static lv_obj_t *create_device_control_screen(void) {
  log_i("Creating Device Control screen...");
  style_init();
  if (dvice_control_settings.attr.is_loaded) {
    log_i("Device Control screen loaded");
  } else {
    // 注册时加载失败，使用默认配置
    dvice_control_settings.attr.is_loaded = true;
    dvice_control_settings.attr.is_dirty = true;
    dvice_control_settings.count = 1;
    dvice_control_settings.configs =
        sys_malloc(SYS_MEM_INTERNAL, sizeof(app_config_t));
    if (dvice_control_settings.configs) {
      memset(dvice_control_settings.configs, 0, sizeof(app_config_t));
      // 初始化默认配置
      dvice_control_settings.configs[0].key = UI_DISPLAY_MODE_KEY;
      dvice_control_settings.configs[0].type = APP_CONFIG_TYPE_INT;
      dvice_control_settings.configs[0].Int = UI_FULL_MODE; //or:UI_COMPACT_MODE
      app_settings_update("DevControl", &dvice_control_settings);
    }
  }

  lv_obj_t *screen = lv_obj_create(NULL);
  // 创建 Tabview 作为app根容器
  lv_obj_t *tabview =
      lv_tabview_create(screen, LV_DIR_BOTTOM, scr_act_height() / 12);
  style_tabview_simple(tabview, style_get_base_default(),
                       style_get_base_checked());

  // 创建三个页面
  create_main(tabview); // 主页面(设备控制)
  create_add(tabview);  // 添加设备
  create_user(tabview); // 用户信息

  return screen;
}

/**
 * @brief 销毁 dvice_control 屏幕
 */
static void destroy_device_control_screen(struct app_t *app) {
  log_i("Destroying Device Control screen...");
  style_deinit();
  controller_clear_ui_map(); // 清除设备控件映射表
}

/**
 * @brief 暂停 dvice_control 屏幕
 */
static void pause_device_control_screen(struct app_t *app) {
  log_d("Device Control screen paused.");
}

/**
 * @brief 恢复 dvice_control 屏幕
 */
static void resume_device_control_screen(struct app_t *app) {
  log_d("Device Control screen resumed.");
}
