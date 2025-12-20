#include "Colorwheel/UI/screens/ui_screen_colorwheel.h"
#include "app_manager.h"
#include "core_app.h"

#define LOG_TAG "ColorWheelApp"
#include "elog.h"

// 图标声明（需要在 assets 中提供）
LV_IMG_DECLARE(icon_colorwheel);

// Forward declarations
static lv_obj_t *create_colorwheel_screen(void);
static void resume_colorwheel_screen(struct app_t *app);

// App definition
static app_def_t colorwheel_app_def = {
    .name = "ColorWheel",
    .icon = &icon_colorwheel,
    .create = create_colorwheel_screen,
    .destroy = NULL, // Rely on LVGL automatic deletion
    .pause = NULL,
    .resume = resume_colorwheel_screen
};

/**
 * @brief 注册 ColorWheel 应用
 */
void colorwheel_app_register(void) {
    app_manager_register(&colorwheel_app_def);
}

/**
 * @brief 创建 ColorWheel 屏幕
 */
static lv_obj_t *create_colorwheel_screen(void) {
    if (!ui_screen_colorwheel) {
        ui_screen_colorwheel_init();
    }
    
    log_d("ColorWheel screen created");
    return ui_screen_colorwheel;
}

/**
 * @brief 恢复 ColorWheel 屏幕
 */
static void resume_colorwheel_screen(struct app_t *app) {
    // 可以在这里添加恢复特定状态的逻辑
    log_d("ColorWheel screen resumed");
}