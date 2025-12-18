#include "util.h"

/**
 * @brief 创建自定义样式滑块
 * @param color  颜色
 * @param Screens 
 * @return 
 */
lv_obj_t* slider_create(lv_color_t color, lv_obj_t* parent) {
    lv_obj_t* slider = lv_slider_create(parent);                      //创建滑块部件
    lv_obj_set_size(slider, scr_act_width() / 3, scr_act_height() / 20);    //设置滑块大小

    lv_obj_set_style_bg_color(slider, color, LV_PART_INDICATOR);            //设置滑块"指示器"颜色
    lv_obj_set_style_bg_color(slider, color, LV_PART_MAIN);                 //设置滑块"主体"颜色
    lv_obj_set_style_bg_opa(slider, 100, LV_PART_MAIN);                     //设置滑块"主体"透明度(0~255,0为全透明,255为不透明)

    lv_obj_remove_style(slider, NULL, LV_PART_KNOB);                        //移除手柄部分

    return slider;
}

//关闭切换动画回调
void anim_scroll_disable_cb(lv_event_t* e)
{
    // 从事件参数中获取动画描述符
    lv_anim_t* a = (lv_anim_t*)lv_event_get_param(e);
    // 检查动画描述符是否存在，并将其动画时间设置为 0
    if (a) { a->time = 0;}
}


//网格布局测试
void _test_layout_grid_(lv_obj_t* grid_obj, uint8_t row_num, uint8_t col_num) {
    uint8_t row, col;
    for (uint8_t row = 0; row < row_num; row++) {
        // 遍历每一列
        for (uint8_t col = 0; col < col_num; col++) {
            // 1. 创建一个简单的占位符对象
            lv_obj_t* placeholder = lv_obj_create(grid_obj);
            lv_obj_clear_flag(placeholder, LV_OBJ_FLAG_CLICKABLE);//移除点击属性
            // 为其设置一个独特的背景色
            lv_obj_set_style_bg_color(placeholder, lv_palette_main(LV_PALETTE_RED + row), 0);
            lv_obj_set_style_border_width(placeholder, 1, 0); // 加个边框看得更清楚
            lv_obj_set_style_border_color(placeholder, lv_color_white(), 0);
            lv_obj_set_style_bg_opa(placeholder, LV_OPA_10, 0);
            // 3. 在占位符中央添加一个标签，显示其坐标
            lv_obj_t* label = lv_label_create(placeholder);
            lv_label_set_text_fmt(label, "%d, %d", row, col);
            lv_obj_set_style_text_color(label, lv_color_white(), 0);
            lv_obj_center(label);
            // 4. 将占位符放置在正确的网格单元格中，并让它填满整个单元格
            lv_obj_set_grid_cell(placeholder,
                LV_GRID_ALIGN_STRETCH, // 水平方向拉伸
                col,                   // 列索引
                1,                     // 占 1 列
                LV_GRID_ALIGN_STRETCH, // 垂直方向拉伸
                row,                   // 行索引
                1);                    // 占 1 行
        }
    }
}


