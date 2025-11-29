/*
 * gpio_led.h
 *
 *  Created on: Nov 29, 2025
 *      Author: 12114
 */

#ifndef HAL_GPIO_LED_GPIO_LED_H_
#define HAL_GPIO_LED_GPIO_LED_H_

#include "led_hal.h"
// 前向声明
typedef struct gpio_led_t gpio_led_t;

typedef struct{
    led_hal_vtable_t base_vtable;
    void (*toggle)(gpio_led_t *self);
} gpio_led_vtable_t;

// 定义led类结构体
struct gpio_led_t {
	led_hal_t base;

    // led类成员变量
    gpio_driver_t* driver;  //依赖的驱动
    uint8_t active_level;	//私有变量
};

void gpio_led_init(gpio_led_t *self, gpio_driver_t *driver, uint8_t active_level);
gpio_led_t *gpio_led_create(gpio_driver_t *driver, uint8_t active_level);
void gpio_led_delete(gpio_led_t *self);

void gpio_led_on(led_hal_t *self);
void gpio_led_off(led_hal_t *self);
void gpio_led_set_state(led_hal_t *self, uint32_t data);
uint32_t gpio_led_get_state(led_hal_t *self);


#endif /* HAL_GPIO_LED_GPIO_LED_H_ */
