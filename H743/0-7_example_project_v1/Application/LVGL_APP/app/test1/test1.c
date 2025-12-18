#include "test1.h"

#ifndef NDEBUG
LV_IMG_DECLARE(wallpaper);//壁纸
#endif

LV_IMG_DECLARE(icon_test1);//图标声明

#ifndef NDEBUG
void test1_test(void) {
    lv_obj_t* home_wallpaper = lv_obj_create(lv_scr_act());
    lv_obj_set_size(home_wallpaper, 800, 480); // 设置为屏幕大小
    lv_obj_set_style_pad_all(home_wallpaper, 0, LV_PART_MAIN);//内边距
    lv_obj_set_style_border_width(home_wallpaper, 0, LV_PART_MAIN);//边框
    //这里先使用图片作为壁纸,省内存也可用API绘制
    lv_obj_t* bg_img = lv_img_create(home_wallpaper);       /*样式修改标记*/
    lv_img_set_src(bg_img, &wallpaper);

    //切换屏幕
    lv_obj_t* test1_Screens = test1_create_cb();
    lv_scr_load_anim(test1_Screens, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 1200, false);

}
#endif

//---------- 接口实现 -------------//
static app_def_t
test1 = {
    .create = test1_create_cb,
    .destroy = test1_destroy_cb,
    .pause = test1_pause_cb,
    .resume = test1_resume_cb,
    .name = "test1",
    .icon = &icon_test1
};

app_def_t* test1_def_get() {
    return &test1;
}

lv_obj_t* test1_create_cb(void) {
#ifdef NDEBUG
    printf("test1_create");
#endif
    lv_obj_t* test1_Screens;
    test1_Screens = lv_obj_create(NULL);
    lv_obj_clean(test1_Screens);//清屏

    lv_obj_t* obj = lv_label_create(test1_Screens);
    lv_label_set_text(obj, "test1");
    lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_STATE_DEFAULT);
    lv_obj_center(obj);

    return test1_Screens;
}

lv_obj_t* test1_destroy_cb(struct activity_t* activity) {

}

lv_obj_t* test1_pause_cb(struct activity_t* activity) {

}
lv_obj_t* test1_resume_cb(struct activity_t* activity) {

}
