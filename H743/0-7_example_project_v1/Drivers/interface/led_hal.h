/*
 * led_hal.h
 *
 *  Created on: Nov 29, 2025
 *      Author: 12114
 */

#ifndef HAL_LED_HAL_LED_HAL_H_
#define HAL_LED_HAL_LED_HAL_H_

// 前向声明
typedef struct led_hal_t led_hal_t;

// 定义虚函数表类型
typedef struct{
    void (*on)(led_hal_t *self);
    void (*off)(led_hal_t *self);
    void (*set_data)(led_hal_t *self, uint32_t data);
    uint32_t (*get_data)(led_hal_t *self);
} led_hal_vtable_t;

// 纯虚类,定义led_hal行为
struct led_hal_t{
    led_hal_vtable_t *vtable;
};


#endif /* HAL_LED_HAL_LED_HAL_H_ */
