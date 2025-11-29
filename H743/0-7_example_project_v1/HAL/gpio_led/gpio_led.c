/*
 * led.c
 *
 *  Created on: Nov 25, 2025
 *      Author: 12114
 */
#include <stdlib.h>
#include "gpio_led.h"

/* ==========================================
 * 默认实现 (Private / Protected)
 * ========================================== */
static void _gpio_led_on(led_hal_t *self){
    if (self && self->driver)
        GPIO_WRITE(self->driver, self->active_level);
}

static void _gpio_led_off(led_hal_t *self){
    if (self && self->driver)
        GPIO_WRITE(self->driver, !self->active_level);
}

static uint32_t _gpio_led_set_state(led_hal_t *self, uint32_t data){
    if (self && self->driver)
        return GPIO_WRITE(self->driver, data?self->active_level:!self->active_level);
}

static uint32_t _gpio_led_get_state(led_hal_t *self){
    if (self && self->driver)
        return GPIO_READ(self->driver)?self->active_level:!self->active_level;
}

// LED 类虚函数表实例(实现接口行为)
led_hal_vtable_t gpio_led_hal_vtable = {
    .on = _gpio_led_on,
    .off = _gpio_led_off,
    .set_data = _gpio_led_set_state,
    .get_data = _gpio_led_get_state
};

/* ==========================================
 * 公共 API 实现 (Dispatch Layer)
 * ========================================== */
void gpio_led_on(led_hal_t *self){
    if (self && self->vtable && self->vtable->on)
        LED_ON(self);    
}

void gpio_led_off(led_hal_t *self){
    if (self && self->vtable && self->vtable->off)
        LED_OFF(self);
}

void gpio_led_set_state(led_hal_t *self, uint32_t data){
    if (self && self->vtable && self->vtable->change)
        LED_CHANGE(self);
}

uint32_t gpio_led_get_state(led_hal_t *self){
    if (self && self->vtable && self->vtable->get_state)
        return LED_GET_STATE(self);
    return 0;
}

/* ==========================================
 * 构造与初始化
 * ========================================== */
void gpio_led_init(gpio_led_t *self, gpio_driver_t *driver, uint8_t active_level){
    self->base.vtable = &gpio_led_hal_vtable;
    self->driver = driver;
    self->active_level = active_level; 
}

gpio_led_t *gpio_led_create(gpio_driver_t *driver, uint8_t active_level){
    gpio_led_t *self = (gpio_led_t *)malloc(sizeof(gpio_led_t));
    if (self){
        gpio_led_init(self, driver, active_level);
    }
    return self;
}

void gpio_led_delete(gpio_led_t *self){
    if(self){
        free(self);
    }
}
