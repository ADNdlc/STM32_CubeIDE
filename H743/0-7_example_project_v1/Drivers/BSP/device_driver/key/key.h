/*
 * Key.h
 *
 *  Created on: May 23, 2025
 *      Author: 12114
 */

#ifndef __Key_H
#define __Key_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32H7xx_hal.h"

typedef struct Button Button;

// 按键事件回调函数类型
typedef void (*ButtonEventCallback)(Button*);

struct Button {
    GPIO_TypeDef* GPIOx;         // GPIO端口
    uint16_t GPIO_Pin;           // GPIO引脚
    uint8_t active_level;        // 有效电平(0:低电平有效 1:高电平有效)

    // 内部状态
    uint8_t last_state;          // 上一次稳定状态
    uint8_t current_state;       // 当前稳定状态
    uint32_t last_check_time;    // 最后检测时间
    uint32_t press_start_time;   // 按下开始时间
    uint32_t release_time;       // 释放时间

    uint8_t click_count;         // 连续点击计数
    uint8_t long_press_flag;     // 长按标记

    // 时间参数
    uint16_t debounce_ms;        // 消抖时间(ms)
    uint16_t long_press_ms;      // 长按判定时间(ms)
    uint16_t click_timeout_ms;   // 连击超时时间(ms)

    /*	点击事件回调
     * 	若需扩展只需新增 ButtonEventCallback指针 然后在Button_Update的 "连击超时检测" 中添加新的case
     *	记得在Button_Init将新添加的回调初始化为NULL
     */
    ButtonEventCallback SinglePressHandler;
    ButtonEventCallback LongPressHandler;
    ButtonEventCallback DoublePressHandler;
    ButtonEventCallback TriplePressHandler;

};

/* 函数声明 */
void Button_Set(Button* btn, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t active_level);
void Button_Update(Button* btn);

void Button_SetDebounce(Button* btn, uint16_t debounce_ms);
void Button_SetLongPress(Button* btn, uint16_t long_press_ms);
void Button_SetClickTimeout(Button* btn, uint16_t timeout_ms);

/*使用方法

1.实例化一个或多个按钮
	Button btn1;	需要配置上输入下/拉模式(根据有效电平)


2.然后需自己实现点击事件

void btn1_single_click(Button* btn) {
    // 单击处理
	printf("btn1_single_click");
}

void btn1_double_click(Button* btn) {
    // 双击处理

}

void btn1_triple_click(Button* btn) {
    // 三击处理

}
void btn1_long_click(Button* btn) {
    // 长按处理

}

3.在初始化后手动绑定(不需要自己调用)

  Button_Init(&btn1,btn1_GPIO_Port,btn1_Pin,GPIO_PIN_RESET);//初始化，指定按钮实例，端口，引脚，有效电平
  btn1.SinglePressHandler = btn1_single_click;
  btn1.DoublePressHandler = btn1_double_click;
  btn1.TriplePressHandler = btn1_triple_click;
  btn1.LongPressHandler = btn1_long_click;

*/

#ifdef __cplusplus
}
#endif

#endif /* __Key_H */
