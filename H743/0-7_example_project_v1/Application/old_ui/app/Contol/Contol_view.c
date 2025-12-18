#include "Contol_view.h"
#include "style.h" // 引入视图头文件以获取样式
#include "util_contol.h"
#include "Contol_controller.h" // 引入控制器以绑定事件
#include <stdio.h>

LV_IMG_DECLARE(default_user);
//
extern void controller_register_ui_control(const char* deviceID, const char* propID, lv_obj_t* obj);
extern void generic_control_event_cb(lv_event_t* e);

//主页网格布局定义(静态)
static lv_coord_t main_row_dsc[] = { 30, 160, 160, 160, 160, LV_GRID_TEMPLATE_LAST }; //高
static lv_coord_t main_col_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST }; //宽


// 释放控件上下文空间的事件回调
static void free_context_event_cb(lv_event_t* e) {
    control_event_ctx_t* ctx = (control_event_ctx_t*)lv_event_get_user_data(e);
    if (ctx) {
        // 释放之前为这个控件分配的上下文内存
        printf("[Info]free_context_event_cb: free context for [Dev='%s'] [Prop='%s']\r\n", ctx->deviceID, ctx->propID);
        lv_mem_free(ctx);
    } else {
        printf("[Warning]free_context_event_cb: ctx is NULL\r\n");
    }
}

void create_main(lv_obj_t* tabview) {
    
    lv_obj_t* tab_main = lv_tabview_add_tab(tabview, LV_SYMBOL_HOME " HOME");
    lv_obj_set_grid_dsc_array(tab_main, main_col_dsc, main_row_dsc);//主页使用网格布局

    //_test_layout_grid_(tab_main, 5, 3);//布局测试

    controller_init_main_tab(tab_main); //根据用户设置填充内容
}

void create_add(lv_obj_t* tabview) {
    lv_obj_t* tab_add = lv_tabview_add_tab(tabview, LV_SYMBOL_PLUS" ADD");
    
    lv_obj_t* add_directory = lv_tabview_create(tab_add, LV_DIR_LEFT, scr_act_width() / 6);//内部使用tableview
    lv_obj_align(add_directory, LV_ALIGN_LEFT_MID, -scr_act_width()/40, 0); //对齐屏幕边缘
    lv_obj_set_size(add_directory, LV_PCT(100), LV_PCT(100));// 设置 add_directory 占满整个父容器

    style_tabview_simple(add_directory, style_get_addtab_default(), style_get_addtab_checked());

    //添加设备分类目录
    lv_obj_t* directory_light = lv_tabview_add_tab(add_directory, "Light");
    lv_obj_t* directory_sensor = lv_tabview_add_tab(add_directory, "Sensor");
    lv_obj_t* test1 = lv_tabview_add_tab(add_directory, "test1");
    lv_obj_t* test2 = lv_tabview_add_tab(add_directory, "test2");
    lv_obj_t* test3 = lv_tabview_add_tab(add_directory, "test3");


    lv_obj_t* btns = lv_tabview_get_tab_btns(add_directory);

    controller_init_add_tab(add_directory); //根据"可添加设备"填充内容
}

void create_user(lv_obj_t* tabview) {
    char* user_name = "user_name"; //假设获取到了登陆的用户id
    char* uid_str = "1234567890";

    lv_obj_t* tab_user = lv_tabview_add_tab(tabview, LV_SYMBOL_SETTINGS" MY");
    lv_obj_clear_flag(tab_user, LV_OBJ_FLAG_SCROLLABLE);// 禁用容器的滚动功能

    lv_obj_t* user = lv_obj_create(tab_user);
    lv_obj_align(user, LV_ALIGN_TOP_MID, 0, 10); // 顶部居中,有一定间距
    lv_obj_set_size(user, scr_act_width() / 2, scr_act_height() / 6);

    lv_obj_t* img = lv_img_create(user);
    lv_img_set_src(img, &default_user);
    lv_img_set_zoom(img, 450);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 10, 3);

    lv_obj_t* name = lv_label_create(user);
    lv_label_set_text(name, user_name);
    lv_obj_set_style_text_font(name, &lv_font_montserrat_20, LV_PART_MAIN); //设置字体
    lv_obj_align_to(name, img, LV_ALIGN_OUT_RIGHT_TOP, scr_act_width() / 20, -10); // 靠右对齐图片

    lv_obj_t* uid = lv_label_create(user);
    lv_label_set_text(uid, uid_str);
    lv_obj_set_style_text_font(uid, &lv_font_montserrat_16, LV_PART_MAIN); //设置字体
    lv_obj_align_to(uid, img, LV_ALIGN_OUT_RIGHT_BOTTOM, scr_act_width() / 20, 0); // 靠右对齐图片

    // 添加包含Setting, about, help, version 按钮的列表
    //列表,添加圆角,单列
    lv_obj_t* list = lv_list_create(tab_user);   //创建表格
    lv_obj_set_size(list, scr_act_width() / 2, scr_act_height() / 3 + 10);
    lv_obj_align_to(list, user, LV_ALIGN_OUT_BOTTOM_MID, 0, 15); // 与用户信息块有一定间距
    //函数第一个参数为添加到的列表, 第二个参数为"按钮图标"(可使用lvgl提供的图标枚举)，第三个参数为"按钮文本"。返回值为添加的按钮
    lv_obj_t* Setting = lv_list_add_btn(list, LV_SYMBOL_SETTINGS, "Setting                                                          "LV_SYMBOL_RIGHT);
    lv_obj_t* about = lv_list_add_btn(list, LV_SYMBOL_SETTINGS, "about                                                            "LV_SYMBOL_RIGHT);
    lv_obj_t* help = lv_list_add_btn(list, LV_SYMBOL_SETTINGS, "help                                                               "LV_SYMBOL_RIGHT);
    lv_obj_t* version = lv_list_add_btn(list, LV_SYMBOL_SETTINGS, "version                                                          "LV_SYMBOL_RIGHT);

    // 添加登出按钮,白色按钮，红色字体,去除阴影
    lv_obj_t* logout_btn = lv_btn_create(tab_user);
    lv_obj_set_size(logout_btn, scr_act_width() / 2, scr_act_height() / 10);
    lv_obj_set_style_bg_color(logout_btn, lv_color_hex(0xffffff), LV_PART_MAIN); // 白色背景
    lv_obj_set_style_shadow_width(logout_btn, 0, LV_PART_ITEMS);  //去除按钮阴影

    lv_obj_align_to(logout_btn, list, LV_ALIGN_OUT_BOTTOM_MID, 0, 10); // 与表格有一定距
    lv_obj_t* logout_label = lv_label_create(logout_btn);
    lv_label_set_text(logout_label, "Logout");
    lv_obj_center(logout_label);
    lv_obj_set_style_text_font(logout_label, &lv_font_montserrat_24, LV_PART_MAIN); //设置字体
    lv_obj_set_style_text_color(logout_label, lv_color_hex(0xFF6045), LV_PART_MAIN); // 红色字体


    controller_init_user_tab(tab_user);
}



/**
 * @brief 创建一个设备卡片
 * 
 * @param parent 父容器
 * @param device 设备数据体
 * @param row 网格行位置
 * @param col 网格列位置
 * 
 * @return 设备控制卡片对象
 */
lv_obj_t* view_create_device_card(lv_obj_t* parent, const struct device_data_t* device, uint8_t row, uint8_t col) {
    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_size(card, LV_PCT(100), LV_PCT(100));
    // 设置卡片的样式，如大小、圆角、阴影
    lv_obj_set_style_radius(card, 10, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(card, 5, LV_PART_MAIN);
    
    //添加到网格布局
    lv_obj_set_grid_cell(card, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);

    lv_obj_update_layout(card); // 更新布局信息
    //获取容器大小
    lv_coord_t width = lv_obj_get_width(card);
    lv_coord_t height = lv_obj_get_height(card);

    // 根据设备数据显示内容
    lv_obj_t* name_label = lv_label_create(card);
    lv_label_set_text(name_label, device->custom_name); // 显示用户自定义的名字
    lv_obj_align(name_label, LV_ALIGN_TOP_RIGHT, 0, 0);

    // 显示设备图标
    lv_obj_t* card_img = lv_img_create(card);
    lv_img_set_src(card_img, device->dev_img); // 使用设备的图标
    lv_img_set_zoom(card_img, 400);
    lv_obj_align(card_img, LV_ALIGN_LEFT_MID, 10, 0);

    // 根据设备属性动态添加控件
    for (int i = 0; i < device->property_count && i < 3; i++) {
        const device_property_t* prop = &device->properties[i];

        //分配上下文空间
        control_event_ctx_t* ctx = (control_event_ctx_t*)lv_mem_alloc(sizeof(control_event_ctx_t));
        strcpy(ctx->deviceID, device->deviceID);
        strcpy(ctx->propID, prop->id);

        switch (prop->type) {
        case PROP_TYPE_SWITCH: {
            lv_obj_t* sw = lv_switch_create(card);

            ctx->target_obj = sw; // 绑定控件指针

            lv_obj_t* label_sw = lv_label_create(card);
            lv_label_set_text(label_sw, device->properties[i].id);

            obj_align_card(sw, device->property_count, width, height);

            lv_obj_set_size(sw, width / 4, height / 6);
            lv_obj_add_state(sw, prop->value.b ? LV_STATE_CHECKED : 0);
            lv_obj_align_to(label_sw, sw, LV_ALIGN_OUT_TOP_LEFT, 0, 0);

            // 绑定事件回调
            lv_obj_add_event_cb(sw, generic_control_event_cb, LV_EVENT_VALUE_CHANGED, ctx);// 绑定事件回调
            lv_obj_add_event_cb(sw, free_context_event_cb, LV_EVENT_DELETE, ctx);   // 删除回调,释放上下文空间

            controller_register_ui_control(device->deviceID, prop->id, sw);// 注册控件到映射表
            break;
        }
        case PROP_TYPE_SLIDER: {
            lv_obj_t* sld = slider_create(lv_color_hex(0xFFE4B5), card);

            ctx->target_obj = sld; // 绑定控件指针

            lv_obj_t* label_sld = lv_label_create(card);
            lv_label_set_text(label_sld, device->properties[i].id);

            obj_align_card(sld, device->property_count, width, height);

            lv_obj_set_size(sld, width / 2, height / 10);
            lv_slider_set_range(sld, prop->min, prop->max);
            lv_slider_set_value(sld, prop->value.i, LV_ANIM_OFF);
            lv_obj_align_to(label_sld, sld, LV_ALIGN_OUT_TOP_LEFT, 0, 0);

            // 绑定事件回调
            lv_obj_add_event_cb(sld, generic_control_event_cb, LV_EVENT_VALUE_CHANGED, ctx);
            lv_obj_add_event_cb(sld, free_context_event_cb, LV_EVENT_DELETE, ctx);

            controller_register_ui_control(device->deviceID, prop->id, sld);// 注册控件到映射表
            break;
        }
        default:

            break;
        }
    }

    return card;
}
