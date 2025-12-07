/*
 * device_mapping.c
 *
 *  Created on: Nov 28, 2025
 *      Author: 12114
 */

#include "device_mapping.h"
#include "usart.h"
#include "tim.h"

// GPIO 设备映射表定义
// 注意：这些值该根据实际硬件连接进行配置

const gpio_mapping_t gpio_mappings[GPIO_MAX_DEVICES] = {
    [GPIO_LED_0] = {GPIOB, GPIO_PIN_1},    		// 开发板led_1(PB0,act=0)
    [GPIO_BUTTON_KEYUP] = {GPIOA, GPIO_PIN_0}, 	// 开发板按键KEY_UP(act=1)
	[GPIO_BUTTON_KEY0] = {GPIOH, GPIO_PIN_3},	// 开发板按键KEY_0 (act=0)
	[GPIO_BUTTON_KEY1] = {GPIOH, GPIO_PIN_2},	// 开发板按键KEY_1 (act=0)
	[GPIO_BUTTON_KEY2] = {GPIOC, GPIO_PIN_13},	// 开发板按键KEY_2 (act=0)
};

// PWM 设备映射表定义
// 注意：这些值应该根据实际硬件连接进行配置
const pwm_mapping_t pwm_mappings[PWM_MAX_DEVICES] = {
    [RGB_LED_RED] = {&htim4, TIM_CHANNEL_1},   // 外接RGBled_R(PD12,act=1)
    [RGB_LED_GREEN] = {&htim4, TIM_CHANNEL_2}, // 外接RGBled_G(PD13,act=1)
    [RGB_LED_BLUE] = {&htim4, TIM_CHANNEL_3},  // 外接RGBled_B(PB8,act=1)
	[PWM_LED_1] = {&htim3, TIM_CHANNEL_3},	   // 开发板led_0(PB1,act=0)
};

// USART 设备映射表定义
const usart_mapping_t usart_mappings[USART_MAX_DEVICES] = {
    [USART_LOGGER] = {&huart1},	// logger输出
    [USART_ATCMD] = {&huart2}
};
