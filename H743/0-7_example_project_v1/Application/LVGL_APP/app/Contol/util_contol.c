#include "util_contol.h"
#include "style.h"

//创建测试卡片



/**
 * @brief 平面化tabview样式(关闭了动画)
 * @param tabview  选项卡对象
 * @param default  默认时btn组件样式
 * @param checked  按下时btn组件样式
 */
void style_tabview_simple(lv_obj_t* tabview, lv_style_t* _default, lv_style_t* _checked) {
    /* ===== 动画 ===== */
    // 关闭切换动画
    lv_obj_t* content = lv_tabview_get_content(tabview);
    lv_obj_add_event_cb(content, anim_scroll_disable_cb, LV_EVENT_SCROLL_BEGIN, NULL);
    // 关闭滑动切换
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    /* ===== 按钮部分 ===== */
    lv_obj_t* btn = lv_tabview_get_tab_btns(tabview);
    //设置样式
    lv_obj_add_style(btn, _default, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_add_style(btn, _checked, LV_PART_ITEMS | LV_STATE_CHECKED);

    //去除选中边框
    lv_obj_set_style_border_width(btn, 0, LV_PART_ITEMS | LV_STATE_CHECKED);
    //取消聚焦
    lv_obj_clear_state(btn, LV_STATE_FOCUS_KEY);
}


void obj_align_card(lv_obj_t* obj, uint8_t count, lv_coord_t width, lv_coord_t height) {
    static uint8_t i;
    i++;
    if (count == 1) {
        lv_obj_align(obj, LV_ALIGN_CENTER, width / 8, height/15);
    }
    else if (count == 2) {
        lv_obj_align(obj, LV_ALIGN_TOP_MID, width / 8, (height/4)*i + 10);
    }
    else{
        lv_obj_align(obj, LV_ALIGN_TOP_MID, width / 8, (height/5)*i + 4);
    }
    if ( i >= count) {i = 0;}
}

void style_gradient_1(lv_obj_t* obj) { //弃用
    lv_obj_t* top_obj = lv_obj_create(obj);
    lv_obj_set_size(top_obj, scr_act_width() / 6, 30);
    lv_obj_align(top_obj, LV_ALIGN_OUT_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(top_obj, lv_color_hex(0xff2222), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(top_obj, 255, LV_PART_MAIN);
    //lv_obj_add_style(top_obj, style_get_gradient_style_top(), LV_PART_MAIN);

    lv_obj_t* bottom_obj = lv_obj_create(obj);
    lv_obj_set_size(bottom_obj, scr_act_width() / 6, 30);
    lv_obj_align(bottom_obj, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    //lv_obj_add_style(bottom_obj, style_get_gradient_style_bottom(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bottom_obj, lv_color_hex(0xff2222), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(top_obj, 255, LV_PART_MAIN);
}



