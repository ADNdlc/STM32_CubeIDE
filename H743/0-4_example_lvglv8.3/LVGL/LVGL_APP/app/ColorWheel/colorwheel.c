#include "ui/Act_Manager.h"
#include "colorwheel.h"

#ifndef NDEBUG
LV_IMG_DECLARE(wallpaper);//壁纸
#endif

LV_IMG_DECLARE(icon_colorwheel);//图标声明

/*  ======================================= 私有变量 ======================================= */
static lv_obj_t* obj;

static lv_obj_t* cw; //颜色轮对象
static lv_obj_t* value; //色相值显示标签

static lv_obj_t* slider_S; //饱和度滑块
static lv_obj_t* slider_V; //明度滑块
static lv_obj_t* lable_S; //饱和度标签
static lv_obj_t* lable_V; //明度标签
static lv_obj_t* value_S; //饱和度值显示
static lv_obj_t* value_V; //明度值显示

static lv_obj_t* slider_r;     //红色通道滑块
static lv_obj_t* slider_g;
static lv_obj_t* slider_b;

static lv_obj_t* lable_r;      //红色通道标签
static lv_obj_t* lable_g;
static lv_obj_t* lable_b;

static lv_obj_t* value_r;      //红色通道值显示
static lv_obj_t* value_g;
static lv_obj_t* value_b;

// 用于在颜色为灰度时保存上一个有效的色相值
static uint16_t last_hue = 0;

/* ======================================= 私有函数 ======================================= */
static void app_update_controls(lv_obj_t* source);// 更新UI函数
static void app_slider_rgb_event_cb(lv_event_t* e);// RGB滑块事件回调
static void app_colorwheel_event_cb(lv_event_t* e);// 色轮事件回调
static void app_slider_hsv_event_cb(lv_event_t* e);// HSV滑块事件回调

/**
 * @brief   创建有颜色的滑块
 * @param  color 滑块颜色
 * @return 滑块对象
 */
lv_obj_t* slider_create(lv_color_t color, lv_obj_t* Screens) {
    lv_obj_t* slider = lv_slider_create(Screens);                      //创建滑块部件
    lv_obj_set_size(slider, scr_act_width() / 3, scr_act_height() / 20);    //设置滑块大小

    lv_obj_set_style_bg_color(slider, color, LV_PART_INDICATOR);            //设置滑块"指示器"颜色
    lv_obj_set_style_bg_color(slider, color, LV_PART_MAIN);                 //设置滑块"主体"颜色
    lv_obj_set_style_bg_opa(slider, 100, LV_PART_MAIN);                     //设置滑块"主体"透明度(0~255,0为全透明,255为不透明)

    lv_obj_remove_style(slider, NULL, LV_PART_KNOB);                        //移除手柄部分

    return slider;
}

/**
 * @brief 创建一个调色程序demo
 * @param Screens 屏幕对象
 */
void colorwheel_demo(lv_obj_t* Screens) {
    cw = lv_colorwheel_create(Screens, false); // 单独显示手柄的颜色
    //设置色环的弧宽度
    lv_obj_set_style_arc_width(cw, 20, LV_PART_MAIN);
    lv_obj_align(cw, LV_ALIGN_CENTER, -scr_act_width() / 4, 0);
    //设置选中颜色
    lv_colorwheel_set_rgb(cw, lv_color_hex(0x00ff00));
    lv_obj_add_event_cb(cw, app_colorwheel_event_cb, LV_EVENT_VALUE_CHANGED, NULL); //设置颜色轮回调,当色轮颜色变化时触发


    //创建一个方块对象,用于显示选中颜色
    obj = lv_obj_create(Screens);
    lv_obj_align(obj, LV_ALIGN_CENTER, -scr_act_width() / 4, 0);
    lv_obj_set_style_bg_color(obj, lv_colorwheel_get_rgb(cw), LV_PART_MAIN);
    //lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, LV_PART_MAIN); //设置方块的圆角(半径:成圆)
    lv_obj_set_style_radius(obj, 20, LV_PART_MAIN);

    //这里色环只用来调节色相，明度饱和度由滑块控制
    lv_colorwheel_set_mode_fixed(cw, true); //设置颜色模式不可变(只是不可通过手柄切换,代码还是可以改变模式的)
    lv_colorwheel_set_mode(cw, LV_COLORWHEEL_MODE_HUE);//色相模式

    /*=========================================================HVS相关====================================================================*/
        //色相值显示
    value = lv_label_create(Screens);                                    //创建色相值显示标签
    lv_obj_align_to(value, cw, LV_ALIGN_OUT_TOP_MID, -12, -12);                 //对齐外右,稍微偏移些
    lv_label_set_text_fmt(value, "H: %d", 120);                                   //设置色相值显示文本(初始为绿色,色相值为120)
    lv_obj_set_style_text_font(value, &lv_font_montserrat_24, LV_PART_MAIN);  //设置色相值显示标签字体

    //由于关闭了手柄颜色变化,所以初始需要手动设置手柄颜色
    lv_obj_set_style_bg_color(cw, lv_color_hex(0x00FF00), LV_PART_KNOB);

    /*饱和度滑块*/
    slider_S = slider_create(lv_color_hex(0x666666), Screens);                                 //创建滑块
    lv_slider_set_range(slider_S, 0, 100);                                             //设置滑块的范围(图片缩放值范围)
    lv_obj_align(slider_S, LV_ALIGN_TOP_MID, scr_act_height() / 3, scr_act_height() * 2 / 3);   //设置滑块位置
    //饱和度标签
    lable_S = lv_label_create(Screens);                                    //创建饱和度标签
    lv_obj_align_to(lable_S, slider_S, LV_ALIGN_OUT_LEFT_MID, 0, -5);            //对齐外左
    lv_label_set_text(lable_S, "S:");                                           //设置饱和度标签文本
    lv_obj_set_style_text_font(lable_S, &lv_font_montserrat_24, LV_PART_MAIN);  //设置饱和度标签字体
    //饱和度值显示
    value_S = lv_label_create(Screens);                                    //创建饱和度值显示标签
    lv_obj_align_to(value_S, slider_S, LV_ALIGN_OUT_RIGHT_MID, 7, -scr_act_height() / 45 + 5);//对齐外右,稍微偏移些
    lv_label_set_text(value_S, "100");                                          //设置饱和度值显示文本
    lv_obj_set_style_text_font(value_S, &lv_font_montserrat_24, LV_PART_MAIN);  //设置饱和度值显示标签字体

    /*明度度滑块*/
    slider_V = slider_create(lv_color_hex(0x999d99), Screens);                                //创建滑块
    lv_slider_set_range(slider_V, 0, 100);                                             //设置滑块的范围
    lv_obj_align_to(slider_V, slider_S, LV_ALIGN_OUT_BOTTOM_LEFT, 0, scr_act_height() / 20);//设置滑块位置
    //明度标签
    lable_V = lv_label_create(Screens);                                    //创建明度标签
    lv_obj_align_to(lable_V, slider_V, LV_ALIGN_OUT_LEFT_MID, 0, -5);            //对齐外左
    lv_label_set_text(lable_V, "V:");                                           //设置明度标签文本
    lv_obj_set_style_text_font(lable_V, &lv_font_montserrat_24, LV_PART_MAIN);  //设置明度标签字体
    //明度值显示
    value_V = lv_label_create(Screens);                                    //创建明度值显示标签
    lv_obj_align_to(value_V, slider_V, LV_ALIGN_OUT_RIGHT_MID, 7, -scr_act_height() / 45 + 5);//对齐外右,稍微偏移些
    lv_label_set_text(value_V, "100");                                          //设置明度值显示文本
    lv_obj_set_style_text_font(value_V, &lv_font_montserrat_24, LV_PART_MAIN);  //设置明度值显示标签字体

    //设置饱和和明度滑块值
    lv_color_hsv_t hsv = lv_colorwheel_get_hsv(cw); //获取色轮当前HSV结构体
    lv_slider_set_value(slider_S, hsv.s, LV_ANIM_OFF); //设置饱和度滑块值
    lv_slider_set_value(slider_V, hsv.v, LV_ANIM_OFF); //设置明度滑块值

    //SVC滑块统一的回调函数
    lv_obj_add_event_cb(slider_S, app_slider_hsv_event_cb, LV_EVENT_VALUE_CHANGED, NULL); //设置饱和度滑块回调
    lv_obj_add_event_cb(slider_V, app_slider_hsv_event_cb, LV_EVENT_VALUE_CHANGED, NULL); //设置明度滑块回调

    /*=====================================================RGB相关========================================================================*/
    //创建RGB三滑块,标签,值显示
    /*红色通道控制滑块*/
    slider_r = slider_create(lv_color_hex(0xff0000), Screens);                                      //创建红滑块
    lv_slider_set_range(slider_r, 0, 255);                                                    //设置滑块的范围(红色通道范围0~255)
    lv_obj_align(slider_r, LV_ALIGN_TOP_MID, scr_act_height() / 3, scr_act_height() / 5);
    //标识
    lable_r = lv_label_create(Screens);                                    //创建红色通道标签
    lv_obj_align_to(lable_r, slider_r, LV_ALIGN_OUT_LEFT_MID, 0, -5);            //对齐外左
    lv_label_set_text(lable_r, "R:");                                           //设置红色通道标签文本
    lv_obj_set_style_text_font(lable_r, &lv_font_montserrat_24, LV_PART_MAIN);  //设置红色通道标签字体
    //通道值
    value_r = lv_label_create(Screens);                                    //创建红色通道值标签
    lv_obj_align_to(value_r, slider_r, LV_ALIGN_OUT_RIGHT_MID, 7, -scr_act_height() / 45 + 5);//对齐外右,稍微偏移些
    lv_label_set_text(value_r, "0");                                           //设置红色通道值标签文本
    lv_obj_set_style_text_font(value_r, &lv_font_montserrat_24, LV_PART_MAIN);  //设置红色通道值标签字体

    /*绿色通道控制滑块*/
    slider_g = slider_create(lv_color_hex(0x00ff00), Screens);
    lv_slider_set_range(slider_g, 0, 255);
    lv_obj_align_to(slider_g, slider_r, LV_ALIGN_OUT_BOTTOM_LEFT, 0, scr_act_height() / 20);
    lv_slider_set_value(slider_g, 255, LV_ANIM_OFF); //设置绿色通道滑块初始值为255
    //标识
    lable_g = lv_label_create(Screens);
    lv_obj_align_to(lable_g, slider_g, LV_ALIGN_OUT_LEFT_MID, 0, -5);
    lv_label_set_text(lable_g, "G:");
    lv_obj_set_style_text_font(lable_g, &lv_font_montserrat_24, LV_PART_MAIN);  //设置绿色通道标签字体
    //通道值
    value_g = lv_label_create(Screens);
    lv_obj_align_to(value_g, slider_g, LV_ALIGN_OUT_RIGHT_MID, 7, -scr_act_height() / 45 + 5); //对齐外右,稍微偏移些
    lv_label_set_text(value_g, "255");                                          //设置绿色通道值标签文本
    lv_obj_set_style_text_font(value_g, &lv_font_montserrat_24, LV_PART_MAIN);  //设置绿色通道值标签字体

    /*蓝色通道控制滑块*/
    slider_b = slider_create(lv_color_hex(0x0000ff), Screens);
    lv_slider_set_range(slider_b, 0, 255);
    lv_obj_align_to(slider_b, slider_g, LV_ALIGN_OUT_BOTTOM_LEFT, 0, scr_act_height() / 20);
    //标识
    lable_b = lv_label_create(Screens);
    lv_obj_align_to(lable_b, slider_b, LV_ALIGN_OUT_LEFT_MID, 0, -5);
    lv_label_set_text(lable_b, "B:");
    lv_obj_set_style_text_font(lable_b, &lv_font_montserrat_24, LV_PART_MAIN);  //设置蓝色通道标签字
    //通道值
    value_b = lv_label_create(Screens);
    lv_obj_align_to(value_b, slider_b, LV_ALIGN_OUT_RIGHT_MID, 7, -scr_act_height() / 45 + 5); //对齐外右,稍微偏移些
    lv_label_set_text(value_b, "0");                                           //设置蓝色通道值标签文本
    lv_obj_set_style_text_font(value_b, &lv_font_montserrat_24, LV_PART_MAIN);  //设置蓝色通道值标签字体

    //RGB滑块统一的回调函数
    lv_obj_add_event_cb(slider_r, app_slider_rgb_event_cb, LV_EVENT_VALUE_CHANGED, NULL); //红色通道滑块回调
    lv_obj_add_event_cb(slider_g, app_slider_rgb_event_cb, LV_EVENT_VALUE_CHANGED, NULL); //绿色通道滑块回调
    lv_obj_add_event_cb(slider_b, app_slider_rgb_event_cb, LV_EVENT_VALUE_CHANGED, NULL); //蓝色通道滑块回调

}

//====================单独测试=======================//
#ifndef NDEBUG
void colorwheel_test(void) {
    lv_obj_t* home_wallpaper = lv_obj_create(lv_scr_act());
    lv_obj_set_size(home_wallpaper, 800, 480); // 设置为屏幕大小
    lv_obj_set_style_pad_all(home_wallpaper, 0, LV_PART_MAIN);//内边距
    lv_obj_set_style_border_width(home_wallpaper, 0, LV_PART_MAIN);//边框
    lv_obj_t* bg_img = lv_img_create(home_wallpaper);           /*样式修改标记*/

    lv_img_set_src(bg_img, &wallpaper);

    //切换屏幕
    lv_obj_t* colorwheel_Screens = colorwheel_create_cb();
    lv_scr_load_anim(colorwheel_Screens, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 1200, false);
    //LV_SCR_LOAD_ANIM_FADE_OUT     淡出
    //LV_SCR_LOAD_ANIM_OVER_LEFT    页面从左移入屏幕
    //LV_SCR_LOAD_ANIM_MOVE_LEFT    整体向左移动
    //LV_SCR_LOAD_ANIM_FADE_IN      淡入
    //LV_SCR_LOAD_ANIM_OUT_LEFT     主页面向左移出屏幕

}
#endif


/* ======================================= 回调函数 ======================================= */
/**
 * @brief 更新屏幕上所有部件的状态(以色轮为准)
 * @param source 谁发起更新(跳过)
 */
static void app_update_controls(lv_obj_t* source) {
    /* 以色轮作为同一数据中心, 更新所有相关控件 */

    // 1. 从色轮获取当前的颜色状态
    lv_color_t color = lv_colorwheel_get_rgb(cw);
    lv_color_hsv_t hsv = lv_colorwheel_get_hsv(cw);

    //当R=G=B时,即灰度色,此时色相无效,使用上次有效的色相值
    // 如果颜色不是灰度色，则更新 last_hue
    if (hsv.s > 5) { // 增加一个小的阈值，避免浮点计算误差
        last_hue = hsv.h;
    }

    // 2. 更新预览方块的颜色
    lv_obj_set_style_bg_color(obj, color, LV_PART_MAIN);

    // 3. 更新色轮手柄的颜色（始终为纯色相）
    lv_obj_set_style_bg_color(cw, lv_color_hsv_to_rgb(last_hue, 100, 100), LV_PART_KNOB);

    // 4. 更新RGB滑块和标签
    if (source != slider_r) {
        lv_slider_set_value(slider_r, color.ch.red, LV_ANIM_OFF);
    }
    lv_label_set_text_fmt(value_r, "%d", color.ch.red);

    if (source != slider_g) {
        lv_slider_set_value(slider_g, color.ch.green, LV_ANIM_OFF);
    }
    lv_label_set_text_fmt(value_g, "%d", color.ch.green);

    if (source != slider_b) {
        lv_slider_set_value(slider_b, color.ch.blue, LV_ANIM_OFF);
    }
    lv_label_set_text_fmt(value_b, "%d", color.ch.blue);


    // 5. 更新S, V滑块和标签
    if (source != slider_S) {
        lv_slider_set_value(slider_S, hsv.s, LV_ANIM_OFF);
    }
    lv_label_set_text_fmt(value_S, "%d", hsv.s);

    if (source != slider_V) {
        lv_slider_set_value(slider_V, hsv.v, LV_ANIM_OFF);
    }
    lv_label_set_text_fmt(value_V, "%d", hsv.v);

    // 6. 更新色相值显示
    if (source != value) {
        lv_label_set_text_fmt(value, "H: %d", last_hue);
    }

}

/**
 * @brief RGB滑块事件回调
 * @param e 目标
 */
static void app_slider_rgb_event_cb(lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target(e);

    // 1. 从三个RGB滑块合成新的颜色
    lv_color_t color = lv_color_make(lv_slider_get_value(slider_r),
        lv_slider_get_value(slider_g),
        lv_slider_get_value(slider_b));

    // 2. 将新颜色设置给色轮（数据中心）
    // 为了处理灰度问题，我们转换为HSV，保留旧的hue，然后再设置回去
    lv_color_hsv_t hsv = lv_color_to_hsv(color);
    if (hsv.s < 5) { // 如果是灰度色
        hsv.h = last_hue; // 使用上一次的有效色相
    }
    lv_colorwheel_set_hsv(cw, hsv);

    // 3. 调用统一的更新函数，更新所有其他控件
    app_update_controls(slider);
}

/**
 * @brief 色轮事件回调
 * @param e 目标
 */
static void app_colorwheel_event_cb(lv_event_t* e) {
    // 色轮是交互的源头，直接调用更新函数
    app_update_controls(cw);
}

/**
 * @brief HSV滑块事件回调
 * @param e 目标
 */
static void app_slider_hsv_event_cb(lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target(e);

    // 1. 获取色轮当前的HSV值
    lv_color_hsv_t hsv = lv_colorwheel_get_hsv(cw);

    // 2. 根据是哪个滑块，修改HSV的S或V分量
    if (slider == slider_S) {
        hsv.s = lv_slider_get_value(slider);
    }
    else if (slider == slider_V) {
        hsv.v = lv_slider_get_value(slider);
    }

    // 3. 将修改后的HSV值设置回色轮
    lv_colorwheel_set_hsv(cw, hsv);

    // 4. 调用统一的更新函数，更新所有其他控件
    app_update_controls(slider);
}

/* ======================================= 通用app接口实现 ======================================= */
static app_def_t
colorwheel = {
    .create = colorwheel_create_cb,
    .destroy = colorwheel_destroy_cb,
    .pause = colorwheel_pause_cb,
    .resume = colorwheel_resume_cb,
    .name = "ColorWheel",
    .icon = &icon_colorwheel
};

//接口获取
app_def_t* colorwheel_def_get() {
    return &colorwheel;
}

/**
* @brief 创建UI (返回屏幕对象)
*/
lv_obj_t* colorwheel_create_cb(void) {
#ifndef NDEBUG
    printf("colorwheel_create\r\n");
#endif
    static lv_obj_t* colorwheel_Screens;
    colorwheel_Screens = lv_obj_create(NULL);
    lv_obj_clean(colorwheel_Screens);//清屏
    colorwheel_demo(colorwheel_Screens);
    return colorwheel_Screens;
}

/**
* @brief 销毁UI
*/
void colorwheel_destroy_cb(struct activity_t* activity) {}

/**
* @brief 暂停 (当被新活动覆盖时调用)
*/
void colorwheel_pause_cb(struct activity_t* activity) {}

/**
* @brief 恢复 (当返回到此活动时调用)
*/
void colorwheel_resume_cb(struct activity_t* activity) {}

