/*
 * device_mapping.c
 *
 *  Created on: Nov 28, 2025
 *      Author: 12114
 */

#include "device_mapping.h"

// GPIO 设备映射表定义  
// 注意：这些值应该根据实际硬件连接进行配置     
const gpio_mapping_t gpio_mappings[GPIO_MAX_DEVICES] = {
    [GPIO_LED_RED] = {GPIOB, GPIO_PIN_14},     // 示例：红灯连接到PB14
    [GPIO_LED_GREEN] = {GPIOB, GPIO_PIN_15},   // 示例：绿灯连接到PB15
    [GPIO_LED_BLUE] = {GPIOC, GPIO_PIN_0},     // 示例：蓝灯连接到PC0
    [GPIO_BUTTON_USER] = {GPIOC, GPIO_PIN_13}  // 示例：用户按键连接到PC13
}; 

// PWM 设备映射表定义
// 注意：这些值应该根据实际硬件连接进行配置     
const pwm_mapping_t pwm_mappings[PWM_MAX_DEVICES] = {
    [PWM_LED_RED] = {&htim1, TIM_CHANNEL_1},   // 示例：红灯PWM连接到TIM1_CH1
    [PWM_LED_GREEN] = {&htim1, TIM_CHANNEL_2}, // 示例：绿灯PWM连接到TIM1_CH2
    [PWM_LED_BLUE] = {&htim1, TIM_CHANNEL_3}   // 示例：蓝灯PWM连接到TIM1_CH3
};