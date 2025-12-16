/*
 * util.c
 *
 *  Created on: Dec 16, 2025
 *      Author: 12114
 */

#include "util.h"
#include "lvgl.h"

//网格布局测试
void test_layout_grid(lv_obj_t* grid_obj, uint8_t row_num, uint8_t col_num) {
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
