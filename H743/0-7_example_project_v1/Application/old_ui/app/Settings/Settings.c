#include "Settings.h"


LV_IMG_DECLARE(icon_Settings);//图标声明


//---------- 接口实现 -------------//
static app_def_t Settings = {
    .create = Settings_create_cb,
    .destroy = Settings_destroy_cb,
    .pause = Settings_pause_cb,
    .resume = Settings_resume_cb,
    .name = "Settings",
    .icon = &icon_Settings
};
app_def_t* Settings_def_get() {
    return &Settings;
}

lv_obj_t* Settings_create_cb(void) {
#ifdef NDEBUG
    printf("Settings_create");
#endif
    lv_obj_t* Settings_Screens;
    Settings_Screens = lv_obj_create(NULL);
    lv_obj_clean(Settings_Screens);//清屏

    lv_obj_t* obj = lv_label_create(Settings_Screens);
    lv_label_set_text(obj, "Settings");
    lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_STATE_DEFAULT);
    lv_obj_center(obj);

    return Settings_Screens;
}
lv_obj_t* Settings_destroy_cb(struct activity_t* activity) {}
lv_obj_t* Settings_pause_cb(struct activity_t* activity) {}
lv_obj_t* Settings_resume_cb(struct activity_t* activity) {}
