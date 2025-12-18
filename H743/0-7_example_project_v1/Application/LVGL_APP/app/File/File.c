#include "File.h"

LV_IMG_DECLARE(icon_File);//图标声明


//---------- 接口实现 -------------//
static app_def_t
File = {
    .create = File_create_cb,
    .destroy = File_destroy_cb,
    .pause = File_pause_cb,
    .resume = File_resume_cb,
    .name = "File",
    .icon = &icon_File
};

app_def_t* File_def_get() {
    return &File;
}

lv_obj_t* File_create_cb(void) {
#ifdef NDEBUG
    printf("File_create");
#endif
    lv_obj_t* File_Screens;
    File_Screens = lv_obj_create(NULL);
    lv_obj_clean(File_Screens);//清屏

    lv_obj_t* obj = lv_label_create(File_Screens);
    lv_label_set_text(obj, "File");
    lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_STATE_DEFAULT);
    lv_obj_center(obj);

    return File_Screens;
}

lv_obj_t* File_destroy_cb(struct activity_t* activity) {

}

lv_obj_t* File_pause_cb(struct activity_t* activity) {}
lv_obj_t* File_resume_cb(struct activity_t* activity) {}
