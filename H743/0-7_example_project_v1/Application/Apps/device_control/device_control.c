#include "app_manager.h"
#include "core_app.h"
#include "res_manager.h"
#include "lv_util.h"
#include "UI/screens/Contol_view.h"
#include <stddef.h>

#define LOG_TAG "Dvice_Control"
#include "elog.h"

// 接口函数
static lv_obj_t *create_device_control_screen(void);
static void destroy_device_control_screen(struct app_t *app);
static void pause_device_control_screen(struct app_t *app);
static void resume_device_control_screen(struct app_t *app);

// 定义 Device Control 应用
static app_def_t dvice_control_app_def = {
	.name = "DevControl",
	.icon = NULL, // Set during registration
	.create = create_device_control_screen,
	.destroy = destroy_device_control_screen,
	.pause = pause_device_control_screen,
	.resume = resume_device_control_screen,
};

/**
 * @brief 注册 dvice_control 应用
 * @param page_index 放置页码
 */
void device_control_app_register(int page_index)
{
	dvice_control_app_def.icon = res_get_src(RES_IMG_ICON_CONTROL);
	app_manager_register(&dvice_control_app_def, page_index);
}

/**
 * @brief 创建 dvice_control 屏幕
 */
static lv_obj_t *create_device_control_screen(void)
{
	style_init();

	lv_obj_t *screen = lv_obj_create(NULL);
	// 创建 Tabview 作为app根容器
	lv_obj_t *tabview = lv_tabview_create(screen, LV_DIR_BOTTOM, scr_act_height() / 12);
	style_tabview_simple(tabview, style_get_base_default(),
						 style_get_base_checked());

	create_main(tabview);
	//create_add(tabview);
	//create_user(tabview);

	return screen;
}

/**
 * @brief 销毁 dvice_control 屏幕
 */
static void destroy_device_control_screen(struct app_t *app)
{
	style_deinit();
	//controller_clear_ui_map(); // 清除设备控件映射表
}

/**
 * @brief 暂停 dvice_control 屏幕
 */
static void pause_device_control_screen(struct app_t *app) {}

/**
 * @brief 恢复 dvice_control 屏幕
 */
static void resume_device_control_screen(struct app_t *app) {}
