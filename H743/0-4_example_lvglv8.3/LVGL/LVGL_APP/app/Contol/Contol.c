 #include "Contol.h"


LV_IMG_DECLARE(icon_Contol);//图标声明


//---------- 接口实现 -------------//
static app_def_t Contol = {
    .create = Contol_create_cb,
    .destroy = Contol_destroy_cb,
    .pause = Contol_pause_cb,
    .resume = Contol_resume_cb,
    .name = "Contol",
    .icon = &icon_Contol
};

app_def_t* Contol_def_get() {
    return &Contol;
}

lv_obj_t* Contol_create_cb(void) {
#ifdef NDEBUG
    printf("Contol_create");
#endif
    lv_obj_t* Contol_Screens;
    Contol_Screens = lv_obj_create(NULL);
    lv_obj_clean(Contol_Screens);//清屏

    lv_obj_t* obj = lv_label_create(Contol_Screens);
    lv_label_set_text(obj, "Contol");
    lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_STATE_DEFAULT);
    lv_obj_center(obj);

    return Contol_Screens;
}
lv_obj_t* Contol_destroy_cb(struct activity_t* activity) {}
lv_obj_t* Contol_pause_cb(struct activity_t* activity) {}
lv_obj_t* Contol_resume_cb(struct activity_t* activity) {}

