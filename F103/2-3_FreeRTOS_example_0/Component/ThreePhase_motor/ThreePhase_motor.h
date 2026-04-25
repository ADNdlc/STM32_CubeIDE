/*
 * pwm_led.h
 *
 *  Created on: Nov 26, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_PWM_LED_PWM_LED_H_
#define BSP_DEVICE_DRIVER_PWM_LED_PWM_LED_H_

#include "interface_inc.h"
#include <stdint.h>

typedef struct motor_t motor_t;

// Motor_t 结构体
struct motor_t {
  pwm_driver_t *phase_a;
  pwm_driver_t *phase_b;
  pwm_driver_t *phase_c;
	
  float voltage_limit;    // 电压(力矩)限制
};

// 公共 API
void motor_init(motor_t *self,   pwm_driver_t *phase_a, pwm_driver_t *phase_b, pwm_driver_t *phase_c, float voltage_limit);
motor_t *pwm_led_create(motor_t *self,   pwm_driver_t *phase_a, pwm_driver_t *phase_b, pwm_driver_t *phase_c, float voltage_limit);
void motor_destroy(motor_t *self);

/**
 * @brief 设置motor三相电压
 * @param self motor_t 实例
 */
void motor_phase_voltage_set(motor_t *self, float Va, float Vb, float Vc);


#endif /* BSP_DEVICE_DRIVER_PWM_LED_PWM_LED_H_ */
