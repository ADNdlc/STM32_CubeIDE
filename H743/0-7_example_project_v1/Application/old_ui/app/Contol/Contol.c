#include "Contol.h"
#include "style.h" // 引入视图头文件
#include "Contol_view.h"
#include "Contol_controller.h"


LV_IMG_DECLARE(icon_Contol);//应用图标声明


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



/**
 * @brief  创建app的主体UI(静态部分)
 * @param  NULL
 * @return app根容器对象指针
 */
lv_obj_t* Contol_create_cb(void) {
#ifdef NDEBUG
    printf("APP create _Contol_");
#endif
    style_init();

    lv_obj_t* screen = lv_obj_create(NULL);
    // 创建 Tabview 作为app根容器
    lv_obj_t* tabview = lv_tabview_create(screen, LV_DIR_BOTTOM, scr_act_height() / 12);
    style_tabview_simple(tabview, style_get_base_default(), style_get_base_checked());

    create_main(tabview);
    create_add(tabview);
    create_user(tabview);


    return screen;
}


void Contol_destroy_cb(struct activity_t* activity) {
    style_deinit();
    DeviceManager_Unsubscribe(ui_state_update_cb);  //注销ui注册的观察回调
    controller_clear_ui_map();                      //清除设备控件映射表
}

void Contol_pause_cb(struct activity_t* activity) {

}

void Contol_resume_cb(struct activity_t* activity) {

}
