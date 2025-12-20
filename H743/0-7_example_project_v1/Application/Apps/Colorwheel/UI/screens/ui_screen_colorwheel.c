#include "ui_screen_colorwheel.h"
#include "ui_helpers.h"
#include "app_manager.h"

#define LOG_TAG "ColorWheel"
#include "elog.h"

typedef struct {
    lv_color_hsv_t hsv;
    lv_color_t rgb;
} color_model_t;

static color_model_t color_model = {
    .hsv = {.h = 120, .s = 100, .v = 100},
    .rgb = {.full = 0}
};

// UI Objects
lv_obj_t * ui_screen_colorwheel;
static lv_obj_t* cw; // 颜色轮对象
static lv_obj_t* preview_obj; // 颜色预览对象

// HSV Controls
static lv_obj_t* slider_S; // 饱和度滑块
static lv_obj_t* slider_V; // 明度滑块
static lv_obj_t* label_H; // 色相值显示标签
static lv_obj_t* label_S; // 饱和度标签
static lv_obj_t* label_V; // 明度标签
static lv_obj_t* value_S; // 饱和度值显示
static lv_obj_t* value_V; // 明度值显示

// RGB Controls
static lv_obj_t* slider_r; // 红色通道滑块
static lv_obj_t* slider_g; // 绿色通道滑块
static lv_obj_t* slider_b; // 蓝色通道滑块
static lv_obj_t* label_r;  // 红色通道标签
static lv_obj_t* label_g;  // 绿色通道标签
static lv_obj_t* label_b;  // 蓝色通道标签
static lv_obj_t* value_r;  // 红色通道值显示
static lv_obj_t* value_g;  // 绿色通道值显示
static lv_obj_t* value_b;  // 蓝色通道值显示

// 用于在颜色为灰度时保存上一个有效的色相值
static uint16_t last_hue = 120;

// 回调
static void colorwheel_event_cb(lv_event_t* e);
static void slider_hsv_event_cb(lv_event_t* e);
static void slider_rgb_event_cb(lv_event_t* e);
// 更新
static void update_controls_from_model(void); // 更新ui控件
static void update_model_from_hsv(void);    // 从HSV更新数据模型
static void update_model_from_rgb(void);    // 从RGB更新数据模型

/**
 * @brief 创建自定义样式滑块
 * @param color  颜色
 * @param Screens 
 * @return 
 */
static lv_obj_t* slider_create(lv_color_t color, lv_obj_t* parent) {
    lv_obj_t* slider = lv_slider_create(parent);                      //创建滑块部件
    lv_obj_set_size(slider, scr_act_width() / 3, scr_act_height() / 20);    //设置滑块大小
    lv_slider_set_range(slider, 0, 100);

    lv_obj_set_style_bg_color(slider, color, LV_PART_INDICATOR);            //设置滑块"指示器"颜色
    lv_obj_set_style_bg_color(slider, color, LV_PART_MAIN);                 //设置滑块"主体"颜色
    lv_obj_set_style_bg_opa(slider, 100, LV_PART_MAIN);                     //设置滑块"主体"透明度(0~255,0为全透明,255为不透明)

    lv_obj_remove_style(slider, NULL, LV_PART_KNOB);                        //移除手柄部分

    return slider;
}

/**
 * @brief 初始化 ColorWheel 屏幕
 */
void ui_screen_colorwheel_init(void) {
    // 检查是否已经初始化
    if (ui_screen_colorwheel) {
        return;
    }
    
    ui_screen_colorwheel = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_screen_colorwheel, LV_OBJ_FLAG_SCROLLABLE);

    // 初始化颜色模型
    color_model.rgb = lv_color_hsv_to_rgb(
        color_model.hsv.h, 
        color_model.hsv.s, 
        color_model.hsv.v
    );

    // 创建颜色轮
    cw = lv_colorwheel_create(ui_screen_colorwheel, false);
    lv_obj_set_style_arc_width(cw, 20, LV_PART_MAIN);
    lv_obj_align(cw, LV_ALIGN_CENTER, -scr_act_width() / 4, 0);
    lv_colorwheel_set_hsv(cw, color_model.hsv);
    lv_obj_add_event_cb(cw, colorwheel_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 创建颜色预览方块
    preview_obj = lv_obj_create(ui_screen_colorwheel);
    lv_obj_align(preview_obj, LV_ALIGN_CENTER, -scr_act_width() / 4, 0);
    lv_obj_set_style_radius(preview_obj, 20, LV_PART_MAIN);
    lv_obj_set_style_bg_color(preview_obj, color_model.rgb, LV_PART_MAIN);

    // 设置颜色轮为固定色相模式
    lv_colorwheel_set_mode_fixed(cw, true);
    lv_colorwheel_set_mode(cw, LV_COLORWHEEL_MODE_HUE);

    /*=================================== HSV 控件 ================================*/
    // 色相值显示
    label_H = lv_label_create(ui_screen_colorwheel);
    lv_obj_align_to(label_H, cw, LV_ALIGN_OUT_TOP_MID, -12, -12);
    lv_label_set_text_fmt(label_H, "H: %d", color_model.hsv.h);
    lv_obj_set_style_text_font(label_H, &lv_font_montserrat_24, LV_PART_MAIN);

    // 饱和度滑块
    slider_S = slider_create(lv_color_hex(0x0000ff), ui_screen_colorwheel);
    lv_obj_align(slider_S, LV_ALIGN_TOP_MID, scr_act_height() / 3, scr_act_height() * 2 / 3);
    lv_slider_set_value(slider_S, color_model.hsv.s, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider_S, slider_hsv_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 饱和度标签
    label_S = lv_label_create(ui_screen_colorwheel);
    lv_obj_align_to(label_S, slider_S, LV_ALIGN_OUT_LEFT_MID, 0, -5);
    lv_label_set_text(label_S, "S:");
    lv_obj_set_style_text_font(label_S, &lv_font_montserrat_24, LV_PART_MAIN);

    // 饱和度值显示
    value_S = lv_label_create(ui_screen_colorwheel);
    lv_obj_align_to(value_S, slider_S, LV_ALIGN_OUT_RIGHT_MID, 7, -scr_act_height() / 45 + 5);
    lv_label_set_text_fmt(value_S, "%d", color_model.hsv.s);
    lv_obj_set_style_text_font(value_S, &lv_font_montserrat_24, LV_PART_MAIN);

    // 明度滑块
    slider_V = slider_create(lv_color_hex(0x999d99), ui_screen_colorwheel);
    lv_obj_align_to(slider_V, slider_S, LV_ALIGN_OUT_BOTTOM_LEFT, 0, scr_act_height() / 20);
    lv_slider_set_value(slider_V, color_model.hsv.v, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider_V, slider_hsv_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 明度标签
    label_V = lv_label_create(ui_screen_colorwheel);
    lv_obj_align_to(label_V, slider_V, LV_ALIGN_OUT_LEFT_MID, 0, -5);
    lv_label_set_text(label_V, "V:");
    lv_obj_set_style_text_font(label_V, &lv_font_montserrat_24, LV_PART_MAIN);

    // 明度值显示
    value_V = lv_label_create(ui_screen_colorwheel);
    lv_obj_align_to(value_V, slider_V, LV_ALIGN_OUT_RIGHT_MID, 7, -scr_act_height() / 45 + 5);
    lv_label_set_text_fmt(value_V, "%d", color_model.hsv.v);
    lv_obj_set_style_text_font(value_V, &lv_font_montserrat_24, LV_PART_MAIN);

    /*=================================== RGB 控件 ================================*/
    // 红色通道滑块
    slider_r = slider_create(lv_color_hex(0xff0000), ui_screen_colorwheel);
    lv_obj_align(slider_r, LV_ALIGN_TOP_MID, scr_act_height() / 3, scr_act_height() / 5);
    lv_slider_set_value(slider_r, color_model.rgb.ch.red, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider_r, slider_rgb_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 红色通道标签
    label_r = lv_label_create(ui_screen_colorwheel);
    lv_obj_align_to(label_r, slider_r, LV_ALIGN_OUT_LEFT_MID, 0, -5);
    lv_label_set_text(label_r, "R:");
    lv_obj_set_style_text_font(label_r, &lv_font_montserrat_24, LV_PART_MAIN);

    // 红色通道值显示
    value_r = lv_label_create(ui_screen_colorwheel);
    lv_obj_align_to(value_r, slider_r, LV_ALIGN_OUT_RIGHT_MID, 7, -scr_act_height() / 45 + 5);
    lv_label_set_text_fmt(value_r, "%d", color_model.rgb.ch.red);
    lv_obj_set_style_text_font(value_r, &lv_font_montserrat_24, LV_PART_MAIN);

    // 绿色通道滑块
    slider_g = slider_create(lv_color_hex(0x00ff00), ui_screen_colorwheel);
    lv_obj_align_to(slider_g, slider_r, LV_ALIGN_OUT_BOTTOM_LEFT, 0, scr_act_height() / 20);
    lv_slider_set_value(slider_g, color_model.rgb.ch.green, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider_g, slider_rgb_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 绿色通道标签
    label_g = lv_label_create(ui_screen_colorwheel);
    lv_obj_align_to(label_g, slider_g, LV_ALIGN_OUT_LEFT_MID, 0, -5);
    lv_label_set_text(label_g, "G:");
    lv_obj_set_style_text_font(label_g, &lv_font_montserrat_24, LV_PART_MAIN);

    // 绿色通道值显示
    value_g = lv_label_create(ui_screen_colorwheel);
    lv_obj_align_to(value_g, slider_g, LV_ALIGN_OUT_RIGHT_MID, 7, -scr_act_height() / 45 + 5);
    lv_label_set_text_fmt(value_g, "%d", color_model.rgb.ch.green);
    lv_obj_set_style_text_font(value_g, &lv_font_montserrat_24, LV_PART_MAIN);

    // 蓝色通道滑块
    slider_b = slider_create(lv_color_hex(0x0000ff), ui_screen_colorwheel);
    lv_obj_align_to(slider_b, slider_g, LV_ALIGN_OUT_BOTTOM_LEFT, 0, scr_act_height() / 20);
    lv_slider_set_value(slider_b, color_model.rgb.ch.blue, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider_b, slider_rgb_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 蓝色通道标签
    label_b = lv_label_create(ui_screen_colorwheel);
    lv_obj_align_to(label_b, slider_b, LV_ALIGN_OUT_LEFT_MID, 0, -5);
    lv_label_set_text(label_b, "B:");
    lv_obj_set_style_text_font(label_b, &lv_font_montserrat_24, LV_PART_MAIN);

    // 蓝色通道值显示
    value_b = lv_label_create(ui_screen_colorwheel);
    lv_obj_align_to(value_b, slider_b, LV_ALIGN_OUT_RIGHT_MID, 7, -scr_act_height() / 45 + 5);
    lv_label_set_text_fmt(value_b, "%d", color_model.rgb.ch.blue);
    lv_obj_set_style_text_font(value_b, &lv_font_montserrat_24, LV_PART_MAIN);
    
    // 更新所有控件
    update_controls_from_model();
}

/**
 * @brief 销毁 ColorWheel 屏幕
 */
void ui_screen_colorwheel_deinit(void) {
    if (ui_screen_colorwheel) {
        // 重置所有静态指针
        ui_screen_colorwheel = NULL;
        cw = NULL;
        preview_obj = NULL;
        slider_S = NULL;
        slider_V = NULL;
        label_H = NULL;
        label_S = NULL;
        label_V = NULL;
        value_S = NULL;
        value_V = NULL;
        slider_r = NULL;
        slider_g = NULL;
        slider_b = NULL;
        label_r = NULL;
        label_g = NULL;
        label_b = NULL;
        value_r = NULL;
        value_g = NULL;
        value_b = NULL;
    }
}

/**
 * @brief 更新所有控件显示
 */
static void update_controls_from_model(void) {
    // 检查对象是否有效
    if (!ui_screen_colorwheel) return;
    
    // 更新预览方块颜色
    lv_obj_set_style_bg_color(preview_obj, color_model.rgb, LV_PART_MAIN);
    
    // 更新色轮手柄颜色
    lv_obj_set_style_bg_color(cw, lv_color_hsv_to_rgb(last_hue, 100, 100), LV_PART_KNOB);
    
    // 更新 HSV 控件
    lv_label_set_text_fmt(label_H, "H: %d", last_hue);
    lv_slider_set_value(slider_S, color_model.hsv.s, LV_ANIM_OFF);
    lv_label_set_text_fmt(value_S, "%d", color_model.hsv.s);
    lv_slider_set_value(slider_V, color_model.hsv.v, LV_ANIM_OFF);
    lv_label_set_text_fmt(value_V, "%d", color_model.hsv.v);
    
    // 更新 RGB 控件
    lv_slider_set_value(slider_r, color_model.rgb.ch.red, LV_ANIM_OFF);
    lv_label_set_text_fmt(value_r, "%d", color_model.rgb.ch.red);
    lv_slider_set_value(slider_g, color_model.rgb.ch.green, LV_ANIM_OFF);
    lv_label_set_text_fmt(value_g, "%d", color_model.rgb.ch.green);
    lv_slider_set_value(slider_b, color_model.rgb.ch.blue, LV_ANIM_OFF);
    lv_label_set_text_fmt(value_b, "%d", color_model.rgb.ch.blue);
}

/**
 * @brief 从 HSV 更新模型
 */
static void update_model_from_hsv(void) {
    color_model.rgb = lv_color_hsv_to_rgb(
        color_model.hsv.h,
        color_model.hsv.s,
        color_model.hsv.v
    );
    
    // 当饱和度大于5时，更新 last_hue
    if (color_model.hsv.s > 5) {
        last_hue = color_model.hsv.h;
    }
}

/**
 * @brief 从 RGB 更新模型
 */
static void update_model_from_rgb(void) {
    color_model.hsv = lv_color_to_hsv(color_model.rgb);
    
    // 处理灰度情况
    if (color_model.hsv.s < 5) {
        color_model.hsv.h = last_hue;
        color_model.rgb = lv_color_hsv_to_rgb(
            color_model.hsv.h,
            color_model.hsv.s,
            color_model.hsv.v
        );
    } else {
        last_hue = color_model.hsv.h;
    }
}

/**
 * @brief 色轮事件回调
 */
static void colorwheel_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        color_model.hsv = lv_colorwheel_get_hsv(cw);
        update_model_from_hsv();
        update_controls_from_model();
    }
}

/**
 * @brief HSV 滑块事件回调
 */
static void slider_hsv_event_cb(lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_VALUE_CHANGED) {
        if (slider == slider_S) {
            color_model.hsv.s = lv_slider_get_value(slider);
        } else if (slider == slider_V) {
            color_model.hsv.v = lv_slider_get_value(slider);
        }
        
        update_model_from_hsv();
        lv_colorwheel_set_hsv(cw, color_model.hsv);
        update_controls_from_model();
    }
}

/**
 * @brief RGB 滑块事件回调
 */
static void slider_rgb_event_cb(lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_VALUE_CHANGED) {
        if (slider == slider_r) {
            color_model.rgb.ch.red = lv_slider_get_value(slider);
        } else if (slider == slider_g) {
            color_model.rgb.ch.green = lv_slider_get_value(slider);
        } else if (slider == slider_b) {
            color_model.rgb.ch.blue = lv_slider_get_value(slider);
        }
        
        update_model_from_rgb();
        lv_colorwheel_set_hsv(cw, color_model.hsv);
        update_controls_from_model();
    }
}