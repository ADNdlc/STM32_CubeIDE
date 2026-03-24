#include "Colorwheel/UI/screens/ui_screen_colorwheel.h"
#include "app_manager.h"
#include "core_app.h"
#include <stddef.h>

#include "lvgl_resource.h"

#define LOG_TAG "ColorWheelApp"
#include "elog.h"

// 接口函数
static lv_obj_t *create_colorwheel_screen(void);
static void destroy_colorwheel_screen(struct app_t *app);
static void pause_colorwheel_screen(struct app_t *app);
static void resume_colorwheel_screen(struct app_t *app);

// 定义 ColorWheel 应用
static app_def_t colorwheel_app_def = {.name = "ColorWheel",
                                       .icon = NULL,
                                       .create = create_colorwheel_screen,
                                       .destroy = destroy_colorwheel_screen,
                                       .pause = pause_colorwheel_screen,
                                       .resume = resume_colorwheel_screen};

/**
 * @brief 注册 ColorWheel 应用
 * @param page_index 放置页码
 */
void colorwheel_app_register(int page_index) {
  colorwheel_app_def.icon = res_get_img(RES_IMG_ICON_COLORWHEEL);
  app_manager_register(&colorwheel_app_def, page_index);
}

/**
 * @brief 创建 ColorWheel 屏幕
 */
static lv_obj_t *create_colorwheel_screen(void) {
  ui_screen_colorwheel_init();

  log_d("ColorWheel screen created");
  return ui_screen_colorwheel;
}

/**
 * @brief 销毁 ColorWheel 屏幕
 */
static void destroy_colorwheel_screen(struct app_t *app) {
  ui_screen_colorwheel_deinit();
  log_d("ColorWheel screen destroyed");
}

/**
 * @brief 暂停 ColorWheel 屏幕
 */
static void pause_colorwheel_screen(struct app_t *app) {
  // 可以在这里添加暂停特定状态的逻辑
  log_d("ColorWheel screen paused");
}

/**
 * @brief 恢复 ColorWheel 屏幕
 */
static void resume_colorwheel_screen(struct app_t *app) {
  // 可以在这里添加恢复特定状态的逻辑
  log_d("ColorWheel screen resumed");
}
