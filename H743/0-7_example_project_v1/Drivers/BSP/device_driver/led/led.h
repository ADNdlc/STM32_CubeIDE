/*
 * led.h
 *
 *  Created on: Nov 25, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_LED_LED_H_
#define BSP_DEVICE_DRIVER_LED_LED_H_

#define USE_HAL_DRIVER

#include <stdint.h>
#include "operations.h"

// 前向声明
typedef struct led_t led_t;
typedef struct gpio_operations gpio_operations_t;

// LED状态枚举
typedef enum {
    LED_STATE_OFF = 0,
    LED_STATE_ON
} led_state_t;

// GPIO操作函数指针结构体
struct gpio_operations {
    void (*write_pin)(void* port, uint16_t pin, uint8_t value);
    uint8_t (*read_pin)(void* port, uint16_t pin);
    void (*toggle_pin)(void* port, uint16_t pin);
};

led_t *led_create_with_ops(void* port, uint16_t pin, const gpio_operations_t* ops);
void led_delete(led_t *self);

void led_on(led_t *self);
void led_off(led_t *self);
void led_toggle(led_t *self);
led_state_t led_get_state(led_t *self); // 添加获取LED状态的函数声明

#endif /* BSP_DEVICE_DRIVER_LED_LED_H_ */
