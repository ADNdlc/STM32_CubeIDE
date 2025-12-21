/*
 * device_mapping.c
 *
 *  Created on: Nov 28, 2025
 *      Author: 12114
 */

#include "device_mapping.h"
#include "fmc.h"
#include "ltdc.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"

// GPIO 设备映射表定义
// 注意：这些值该根据实际硬件连接进行配置

const gpio_mapping_t gpio_mappings[GPIO_MAX_DEVICES] = {
    [GPIO_LED_0] = {GPIOB, GPIO_PIN_1},        // 开发板led_1(PB0,act=0)
    [GPIO_BUTTON_KEYUP] = {GPIOA, GPIO_PIN_0}, // 开发板按键KEY_UP(act=1)
    [GPIO_BUTTON_KEY0] = {GPIOH, GPIO_PIN_3},  // 开发板按键KEY_0 (act=0)
    [GPIO_BUTTON_KEY1] = {GPIOH, GPIO_PIN_2},  // 开发板按键KEY_1 (act=0)
    [GPIO_BUTTON_KEY2] = {GPIOC, GPIO_PIN_13}, // 开发板按键KEY_2 (act=0)
    // 触摸屏控制引脚
    [GPIO_TOUCH_RST] = {GPIOI, GPIO_PIN_8}, // 触摸屏 RST 引脚 (PI8)
    [GPIO_TOUCH_INT] = {GPIOH, GPIO_PIN_7}, // 触摸屏 INT 引脚 (PH7)
};

// PWM 设备映射表定义
// 注意：这些值应该根据实际硬件连接进行配置
const pwm_mapping_t pwm_mappings[PWM_MAX_DEVICES] = {
    [RGB_LED_RED] = {&htim4, TIM_CHANNEL_1},   // 外接RGBled_R(PD12,act=1)
    [RGB_LED_GREEN] = {&htim4, TIM_CHANNEL_2}, // 外接RGBled_G(PD13,act=1)
    [RGB_LED_BLUE] = {&htim4, TIM_CHANNEL_3},  // 外接RGBled_B(PB8,act=1)
    [PWM_LED_1] = {&htim3, TIM_CHANNEL_3},     // 开发板led_0(PB1,act=0)
};

// USART 设备映射表定义
const usart_mapping_t usart_mappings[USART_MAX_DEVICES] = {
    [USART_LOGGER] = {&huart1}, // logger输出
    [USART_ATCMD] = {&huart2}};

// LCD 设备映射表定义
const lcd_mapping_t lcd_mappings[LCD_MAX_DEVICES] = {[LCD_MAIN] = {&hltdc}};

// SDRAM 设备映射表定义
const sdram_mapping_t sdram_mappings[SDRAM_MAX_DEVICES] = {
    [SDRAM_MAIN] = {&hsdram1}};

// I2C 软件模拟设备映射表定义
// 触摸屏 I2C 引脚：SCL=PH6, SDA=PI3
const i2c_soft_mapping_t i2c_soft_mappings[I2C_SOFT_MAX_DEVICES] = {
    [I2C_SOFT_TOUCH] =
        {
            .scl_port = GPIOH,
            .scl_pin = GPIO_PIN_6,
            .sda_port = GPIOI,
            .sda_pin = GPIO_PIN_3,
            .delay_us = 2, // 2us延时，约250kHz
        },
};

// 触摸屏设备映射表定义
const touch_mapping_t touch_mappings[TOUCH_MAX_DEVICES] = {
    [TOUCH_MAIN] =
        {
            .i2c_id = I2C_SOFT_TOUCH,
            .rst_gpio_id = GPIO_TOUCH_RST,
            .int_gpio_id = GPIO_TOUCH_INT,
            .i2c_addr_mode = 0x14, // 使用0x14地址模式 (复位时INT=HIGH)
        },
};

// RTC 设备映射表定义
const rtc_mapping_t rtc_mappings[RTC_MAX_DEVICES] = {[RTC_DEVICE_0] = {&hrtc}};
