/*
 * led.c
 *
 *  Created on: Nov 25, 2025
 *      Author: 12114
 */
#include <stdlib.h>
#include "led.h"

// 定义led操作函数指针类型
typedef void (*led_on_func_t)(led_t *self);
typedef void (*led_off_func_t)(led_t *self);
typedef void (*led_toggle_func_t)(led_t *self);
typedef led_state_t (*led_get_state_func_t)(led_t *self);

// 定义led类的虚函数表结构
typedef struct{
    led_on_func_t on;
    led_off_func_t off;
    led_toggle_func_t toggle;
    led_get_state_func_t get_state;
} led_vtable_t;

// 定义led类结构
struct led_t{
    const led_vtable_t *vtable; // 虚函数表指针
    void* port;
    uint16_t pin;
    const gpio_operations_t* ops;
    uint8_t active_level;
};

// led操作实现
void led_on(led_t *self){
    self->ops->write_pin(self->port, self->pin, self->active_level);
}

void led_off(led_t *self){
    self->ops->write_pin(self->port, self->pin, !self->active_level);
}

void led_toggle(led_t *self){
    self->ops->toggle_pin(self->port, self->pin);
}

// 获取LED当前状态
led_state_t led_get_state(led_t *self) {
    uint8_t pin_state = self->ops->read_pin(self->port, self->pin);
    return (pin_state == self->active_level) ? LED_STATE_ON : LED_STATE_OFF;
}

// led类的虚函数表
static const led_vtable_t led_vtable = {
    .off = led_off,
    .on = led_on,
    .toggle = led_toggle,
    .get_state = led_get_state
};

// 初始化函数
void led_init(led_t *self, void* port, uint16_t pin, const gpio_operations_t* ops, uint8_t active_level){
    self->vtable = &led_vtable;
    self->port = port;
    self->pin = pin;
    self->ops = ops;
    self->active_level = active_level;  // 初始化开启电平
}

// 构造函数 - 通用版本
led_t *led_create_with_ops(void* port, uint16_t pin, const gpio_operations_t* ops){
    led_t *self = malloc(sizeof(led_t));
    if (self == NULL)
    {
        return NULL;
    }
    led_init(self, port, pin, ops, 1);  // 默认高电平开启
    return self;
}

// 析构函数
void led_delete(led_t *self){
    if (self == NULL)
    {
        return;
    }
    free(self);
}

