/*
 * Key.c
 *
 *  Created on: May 23, 2025
 *      Author: 12114
 */

#include "../Key/Key.h"

#include "main.h"

// 按键初始化
void Button_Init(Button* btn, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t active_level) {
    btn->GPIOx = GPIOx;
    btn->GPIO_Pin = GPIO_Pin;
    btn->active_level = active_level;

    btn->last_state = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
    btn->current_state = btn->last_state;
    btn->last_check_time = 0;
    btn->press_start_time = 0;
    btn->release_time = 0;
    btn->click_count = 0;
    btn->long_press_flag = 0;

    // 默认参数
    btn->debounce_ms = 10;
    btn->long_press_ms = 1000;
    btn->click_timeout_ms = 300;

    // 初始化回调
    btn->SinglePressHandler = NULL;
    btn->DoublePressHandler = NULL;
    btn->TriplePressHandler = NULL;
    btn->LongPressHandler = NULL;
}

// 设置消抖时间
void Button_SetDebounce(Button* btn, uint16_t debounce_ms) {
    btn->debounce_ms = debounce_ms > 2 ? debounce_ms : 2;
}

// 设置长按时间
void Button_SetLongPress(Button* btn, uint16_t long_press_ms) {
    btn->long_press_ms = long_press_ms > 100 ? long_press_ms : 100;
}

// 设置连击超时时间
void Button_SetClickTimeout(Button* btn, uint16_t timeout_ms) {
    btn->click_timeout_ms = timeout_ms > 100 ? timeout_ms : 100;
}

// 按键状态更新（需循环调用）
void Button_Update(Button* btn) {
    uint32_t now = HAL_GetTick();//获取时基
    uint8_t current_state = HAL_GPIO_ReadPin(btn->GPIOx, btn->GPIO_Pin);//当前状态

    /* 状态变化检测 */
    if (current_state != btn->last_state) {	//状态变化了
        btn->last_check_time = now;			//更新上次时间
        btn->last_state = current_state;	//更新上次状态
        return;
    }

    /* 消抖处理 */
    if ((now - btn->last_check_time) < btn->debounce_ms) {
        return;//度过时间小于消抖时间
    }

    /* 有效状态变化 */
    if (current_state != btn->current_state) {//更新当前状态
        btn->current_state = current_state;

        if (btn->current_state == btn->active_level) {
            // 状态是 按键按下
            btn->press_start_time = now;//获取当前时间作为 按下开始时间
            btn->long_press_flag = 0;	//清零 长按标记
        } else {
            // 状态是 按键释放
            btn->release_time = now;	//获取当前时间作为 松开时间

            // 短按处理（长按时不统计点击次数）
            if (!btn->long_press_flag) {
                btn->click_count++;//累计点击次数
            }
        }
    }

    /* 长按检测 */
    if ((btn->current_state == btn->active_level) &&			//是长 "触发电平"
        ((now - btn->press_start_time) >= btn->long_press_ms) &&//按下开始时间到现在超过，长按时间
        (!btn->long_press_flag)) {								//标志位0
        btn->long_press_flag = 1;
        if (btn->LongPressHandler) {
            btn->LongPressHandler(btn);
        }
        btn->click_count = 0; // 长按后重置点击计数
    }

    /* 连击超时检测 */
    if ((btn->click_count > 0) &&
        ((now - btn->release_time) >= btn->click_timeout_ms)) {
        switch (btn->click_count) {
            case 1:
                if (btn->SinglePressHandler) btn->SinglePressHandler(btn);//若有效，则执行
                break;
            case 2:
                if (btn->DoublePressHandler) btn->DoublePressHandler(btn);
                break;
            case 3:
                if (btn->TriplePressHandler) btn->TriplePressHandler(btn);
                break;
        }
        btn->click_count = 0;
    }
}
