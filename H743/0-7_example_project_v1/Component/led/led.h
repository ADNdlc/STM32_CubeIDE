/*
 * led.h
 *
 *  Created on: Nov 25, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_LED_LED_H_
#define BSP_DEVICE_DRIVER_LED_LED_H_

#include <stdint.h>
#include "gpio_driver.h"

// 前向声明
typedef struct led_t led_t;

// 定义虚函数表类型
typedef struct {
    void (*on)(led_t *self);
    void (*off)(led_t *self);
    void (*toggle)(led_t *self);
    uint8_t (*get_state)(led_t *self);
} led_vtable_t;

// 定义基类结构体
struct led_t {
    const led_vtable_t *vtable; // 虚表指针必须在第一位
    
    // 基类成员变量
    gpio_driver_t* driver;
    uint8_t active_level;
};

// 公共 API
void led_init(led_t *self, gpio_driver_t* driver, uint8_t active_level);
led_t* led_create(gpio_driver_t* driver, uint8_t active_level);
void led_delete(led_t *self);

// 多态调用接口
void led_on(led_t *self);
void led_off(led_t *self);
void led_toggle(led_t *self);
uint8_t led_get_state(led_t *self);

#endif /* BSP_DEVICE_DRIVER_LED_LED_H_ */
