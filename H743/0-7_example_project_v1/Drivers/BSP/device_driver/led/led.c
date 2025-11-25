/*
 * led.c
 *
 *  Created on: Nov 25, 2025
 *      Author: 12114
 */
#include "main.h"
#include <stdlib.h>
#include "led.h"


// 定义led操作函数指针类型
typedef void (*led_on_func_t)(led_t *self);
typedef void (*led_off_func_t)(led_t *self);
typedef void (*led_toggle_func_t)(led_t *self);

// 定义led类的虚函数表结构
typedef struct{
    led_on_func_t on;
    led_off_func_t off;
    led_toggle_func_t toggle;
} led_vtable_t;

// 定义led类结构
typedef struct led_t{
    const led_vtable_t *vtable; // 虚函数表指针
    GPIO_TypeDef *port;
    uint16_t pin;
} led_t;


// led操作实现
void led_on(led_t *self){
    HAL_GPIO_WritePin(self->port, self->pin, GPIO_PIN_SET);
}

void led_off(led_t *self){
    HAL_GPIO_WritePin(self->port, self->pin, GPIO_PIN_RESET);
}

void led_toggle(led_t *self){
    HAL_GPIO_TogglePin(self->port, self->pin);
}

// led类的虚函数表
static const led_vtable_t led_vtable = {
    .off = led_off,
    .on = led_on,
    .toggle = led_toggle
};

// 初始化函数
void led_init(led_t *self, GPIO_TypeDef *port, uint16_t pin){
    self->vtable = &led_vtable;
    self->port = port;
    self->pin = pin;
}
// 构造
led_t *led_create(GPIO_TypeDef *port, uint16_t pin){
    led_t *self = malloc(sizeof(led_t));
    if (self == NULL)
    {
        return NULL;
    }
    led_init(self, port, pin);
    return self;
}
// 析构
void led_delete(led_t *self){
    if (self == NULL)
    {
        return;
    }
    free(self);
}
