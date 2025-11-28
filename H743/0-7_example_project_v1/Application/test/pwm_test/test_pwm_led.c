/*
 * test_pwm_led.c
 *
 *  Created on: Nov 28, 2025
 *      Author: 12114
 */

#include "pwm_factory.h"
#include "pwm_driver.h"

void test_pwm_led(void) {
    // 获取PWM驱动实例
    pwm_driver_t* red_pwm = pwm_driver_get(PWM_LED_RED);
    pwm_driver_t* green_pwm = pwm_driver_get(PWM_LED_GREEN);
    pwm_driver_t* blue_pwm = pwm_driver_get(PWM_LED_BLUE);

    // 启动PWM
    if (red_pwm) {
        PWM_START(red_pwm);
    }

    if (green_pwm) {
        PWM_START(green_pwm);
    }

    if (blue_pwm) {
        PWM_START(blue_pwm);
    }

    // 设置占空比测试
    if (red_pwm) {
        PWM_SET_DUTY(red_pwm, 500);  // 50% 占空比 (假设周期为1000)
    }

    if (green_pwm) {
        PWM_SET_DUTY(green_pwm, 300);  // 30% 占空比
    }

    if (blue_pwm) {
        PWM_SET_DUTY(blue_pwm, 700);  // 70% 占空比
    }
}