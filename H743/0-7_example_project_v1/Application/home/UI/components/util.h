/*
 * util.h
 *
 *  Created on: Dec 16, 2025
 *      Author: 12114
 */

#ifndef UI_COMPONENTS_UTIL_H_
#define UI_COMPONENTS_UTIL_H_

#include "lvgl.h"
/**
 * @brief 这里是DEBUG用的测试函数
 * 
 */

#ifndef NODEBUG

void test_layout_grid(lv_obj_t* grid_obj, uint8_t row_num, uint8_t col_num);


#endif // NODEBUG

#endif /* UI_COMPONENTS_UTIL_H_ */
