/*
 * led.c
 *
 *  Created on: Nov 25, 2025
 *      Author: 12114
 */
#include <stdlib.h>
#include "led.h"

/* ==========================================
 * 默认实现 (Private / Protected)
 * 这些函数是 GPIO LED 的具体行为
 * ========================================== */
static void default_led_on(led_t *self) {
    self->ops->write_pin(self->port, self->pin, self->active_level);
}

static void default_led_off(led_t *self) {
    self->ops->write_pin(self->port, self->pin, !self->active_level);
}

static void default_led_toggle(led_t *self) {
    self->ops->toggle_pin(self->port, self->pin);
}

static uint8_t default_led_get_state(led_t *self) {
    uint8_t pin_state = self->ops->read_pin(self->port, self->pin);
    return (pin_state == self->active_level) ? 1 : 0;
}

// 基类虚函数表
static const led_vtable_t default_vtable = {
    .on = default_led_on,
    .off = default_led_off,
    .toggle = default_led_toggle,
    .get_state = default_led_get_state
};

/* ==========================================
 * 公共 API 实现 (Dispatch Layer)
 * 核心：查表调用，实现多态
 * ========================================== */

void led_on(led_t *self) {
    if(self && self->vtable && self->vtable->on) 
        self->vtable->on(self); 
}

void led_off(led_t *self) {
    if(self && self->vtable && self->vtable->off) 
        self->vtable->off(self);
}

void led_toggle(led_t *self) {
    if(self && self->vtable && self->vtable->toggle) 
        self->vtable->toggle(self);
}

uint8_t led_get_state(led_t *self) {
    if(self && self->vtable && self->vtable->get_state) 
        return self->vtable->get_state(self);
    return 0;
}

/* ==========================================
 * 构造与初始化
 * ========================================== */

void led_init(led_t *self, void* port, uint16_t pin, const led_gpio_ops_t* ops, uint8_t active_level) {
    self->vtable = &default_vtable; // 绑定基类虚表
    self->port = port;
    self->pin = pin;
    self->ops = ops;
    self->active_level = active_level;
}

led_t* led_create(void* port, uint16_t pin, const led_gpio_ops_t* ops, uint8_t active_level) {
    led_t *self = (led_t*)malloc(sizeof(led_t));
    if (self) {
        led_init(self, port, pin, ops, active_level);
    }
    return self;
}

void led_delete(led_t *self) {
    free(self);
}


/* ----- HAL库依赖的具体实现 ----- */
#ifdef _USE_HAL_DRIVER
#include "main.h"

// HAL GPIO操作的具体实现
static void hal_write_pin(void* port, uint16_t pin, uint8_t value) {
    HAL_GPIO_WritePin((GPIO_TypeDef*)port, pin, (GPIO_PinState)value);
}

static void hal_toggle_pin(void* port, uint16_t pin) {
    HAL_GPIO_TogglePin((GPIO_TypeDef*)port, pin);
}

static uint8_t hal_read_pin(void* port, uint16_t pin) {
    return (uint8_t)HAL_GPIO_ReadPin((GPIO_TypeDef*)port, pin);
}



#endif

/* ----- 标准库库依赖的具体实现 ----- */

//...
