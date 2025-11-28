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
    if (self->driver && self->driver->ops && self->driver->ops->gpio_write) {
        self->driver->ops->gpio_write(self->driver, 1);
    }
}

static void default_led_off(led_t *self) {
    if (self->driver && self->driver->ops && self->driver->ops->gpio_write) {
        self->driver->ops->gpio_write(self->driver, 0);
    }
}

static void default_led_toggle(led_t *self) {
    if (self->driver && self->driver->ops && self->driver->ops->gpio_toggle) {
        self->driver->ops->gpio_toggle(self->driver);
    }
}

static uint8_t default_led_get_state(led_t *self) {
    if (self->driver && self->driver->ops && self->driver->ops->gpio_read) {
        return self->driver->ops->gpio_read(self->driver);
    }
    return 0;
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

void led_init(led_t *self, gpio_driver_t* driver, uint8_t active_level) {
    self->vtable = &default_vtable; // 绑定基类虚表
    self->driver = driver;
    self->active_level = active_level;
}

led_t* led_create(gpio_driver_t* driver, uint8_t active_level) {
    led_t *self = (led_t*)malloc(sizeof(led_t));
    if (self) {
        led_init(self, driver, active_level);
    }
    return self;
}

void led_delete(led_t *self) {
    free(self);
}


