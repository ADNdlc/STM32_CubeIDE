/*
 * pwm_led.h
 *
 *  Created on: Nov 26, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_PWM_LED_PWM_LED_H_
#define BSP_DEVICE_DRIVER_PWM_LED_PWM_LED_H_

#include "led.h"

// 1. PWM 硬件抽象接口
typedef struct {
    // 启动 PWM, duty 为 0-100 或 0-1000
    void (*start_pwm)(void* handle, uint32_t channel);
    void (*stop_pwm)(void* handle, uint32_t channel);
    void (*set_duty)(void* handle, uint32_t channel, uint32_t duty);
} pwm_ops_t;

// 2. 子类虚函数表 (扩展)
typedef struct {
    led_vtable_t base_vtable; // 必须放在第一位，继承父类虚表布局
    void (*set_brightness)(struct pwm_led_t* self, uint32_t duty); // 新增虚函数
} pwm_led_vtable_t;

// 3. 子类结构体
typedef struct pwm_led_t {
    led_t base;            // 继承：必须放在第一位
    
    // 子类特有数据
    const pwm_ops_t* pwm_ops;
    void* pwm_handle;      // 例如 TIM_HandleTypeDef*
    uint32_t channel;
    uint32_t current_duty; // 当前亮度缓存
} pwm_led_t;

// 4. 子类构造函数
pwm_led_t* pwm_led_create(void* pwm_handle, uint32_t channel, const pwm_ops_t* ops);

// 5. 子类特有方法
void pwm_led_set_brightness(pwm_led_t* self, uint32_t duty);

#endif /* BSP_DEVICE_DRIVER_PWM_LED_PWM_LED_H_ */
