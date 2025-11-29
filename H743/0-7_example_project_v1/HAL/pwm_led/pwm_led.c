/*
 * pwm_led.c
 *
 *  Created on: Nov 26, 2025
 *      Author: 12114
 */

#include "pwm_led.h"
#include <stdlib.h>

/* ==========================================
 * 默认实现 (Private / Protected)
 * ========================================== */

// 实现子类行为
static void _pwm_led_on(led_hal_t *self){
    pwm_led_t *this = (pwm_led_t *)self;
    if (this && this->pwm_driver && this->pwm_driver->ops && this->pwm_driver->ops->start){
        PWM_START(self);
    }
}

static void _pwm_led_off(led_hal_t *self){
    pwm_led_t *this = (pwm_led_t *)self;
    if (this && this->pwm_driver && this->pwm_driver->ops && this->pwm_driver->ops->stop){
        PWM_STOP(self);
    }
}

static void _pwm_led_change(led_hal_t *self){ 
    pwm_led_t *this = (pwm_led_t *)self;
}

static uint32_t _pwm_led_get_brightness(led_hal_t *self){ 
    pwm_led_t *this = (pwm_led_t *)self;
    
}

/* 设置亮度
 * brightness 亮度值(0~1000)
 */
static void _pwm_led_set_brightness(pwm_led_t *self, uint16_t brightness)
{
    if (self && self->pwm_driver && self->pwm_driver->ops &&
        self->pwm_driver->ops->set_duty){
        // 根据分辨率映射到占空比
        uint32_t duty_max = self->pwm_driver->ops->get_duty_max(self->pwm_driver);
        // 限制亮度值在有效范围内
        if (brightness > 1000)
        {
            brightness = 1000;
        }
        // 计算实际占空比值
        uint32_t duty = ((uint32_t)brightness * duty_max) / 1000;
        // 设置占空比
        self->pwm_driver->ops->set_duty(self->pwm_driver, duty);
        // 更新当前亮度值
        self->current_duty = duty;
    }
}

/*
 * 获取亮度
 */
static uint16_t default_pwm_led_get_brightness(pwm_led_t *self)
{
    return (uint16_t)self->current_duty;
}

// 虚函数表实例
static const pwm_led_vtable_t default_pwm_led_vtable = {
    .on = default_pwm_led_on,
    .off = default_pwm_led_off,
    .set_brightness = default_pwm_led_set_brightness,
    .get_brightness = default_pwm_led_get_brightness};

/* ==========================================
 * 公共 API 实现 (Dispatch Layer)
 * ========================================== */

void pwm_led_on(pwm_led_t *self)
{
    if (self && self->vtable && self->vtable->on)
        self->vtable->on(self);
}

void pwm_led_off(pwm_led_t *self)
{
    if (self && self->vtable && self->vtable->off)
        self->vtable->off(self);
}

void pwm_led_set_brightness(pwm_led_t *self, uint32_t duty)
{
    if (self && self->vtable && self->vtable->set_brightness)
        self->vtable->set_brightness(self, duty);
}

uint16_t pwm_led_get_brightness(pwm_led_t *self)
{
    if (self && self->vtable && self->vtable->get_brightness)
        return self->vtable->get_brightness(self);
    return 0;
}

/* ==========================================
 * 构造与初始化
 * ========================================== */

void pwm_led_init(pwm_led_t *self, uint32_t freq, pwm_driver_t *pwm_driver)
{
    self->vtable = &default_pwm_led_vtable; // 绑定虚表
    self->pwm_driver = pwm_driver;
    self->current_duty = 0;

    // 设置频率
    if (pwm_driver && pwm_driver->ops && pwm_driver->ops->set_freq)
    {
        pwm_driver->ops->set_freq(pwm_driver, freq);
    }
}

pwm_led_t *pwm_led_create(uint32_t freq, pwm_driver_t *pwm_driver)
{
    pwm_led_t *self = (pwm_led_t *)malloc(sizeof(pwm_led_t));
    if (self)
    {
        pwm_led_init(self, freq, pwm_driver);
    }
    return self;
}

void pwm_led_delete(pwm_led_t *self)
{
    free(self);
}