/*
 * led.h
 *
 *  Created on: Nov 25, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_LED_LED_H_
#define BSP_DEVICE_DRIVER_LED_LED_H_

#include <stdint.h>

// 前向声明
typedef struct led_t led_t;

// 定义 GPIO 操作接口 (硬件抽象层)
typedef struct {
    void (*write_pin)(void* port, uint16_t pin, uint8_t value);
    uint8_t (*read_pin)(void* port, uint16_t pin);
    void (*toggle_pin)(void* port, uint16_t pin);
} led_gpio_ops_t;

// 定义虚函数表类型
typedef struct {
    void (*on)(led_t *self);
    void (*off)(led_t *self);
    void (*toggle)(led_t *self);
    uint8_t (*get_state)(led_t *self);
} led_vtable_t;

// 定义基类结构体 (暴露给子类)
struct led_t {
    const led_vtable_t *vtable; // 虚表指针必须在第一位
    
    // 基类成员变量
    void* port;
    uint16_t pin;
    const led_gpio_ops_t* ops;  // 基础 GPIO 操作
    uint8_t active_level;
};

// 公共 API
void led_init(led_t *self, void* port, uint16_t pin, const led_gpio_ops_t* ops, uint8_t active_level);
led_t* led_create(void* port, uint16_t pin, const led_gpio_ops_t* ops, uint8_t active_level);
void led_delete(led_t *self);

// 多态调用接口
void led_on(led_t *self);
void led_off(led_t *self);
void led_toggle(led_t *self);
uint8_t led_get_state(led_t *self);


// HAL库依赖实现
#ifdef USE_HAL_DRIVER
#define _USE_HAL_DRIVER
#include "stm32h7xx_hal.h"
#endif

#endif /* BSP_DEVICE_DRIVER_LED_LED_H_ */
