/*
 * pwm_driver.h
 *
 *  Created on: Nov 27, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_PWM_LED_PWM_DRIVER_H_
#define BSP_DEVICE_DRIVER_PWM_LED_PWM_DRIVER_H_

#include <stdint.h>

// 前向声明
typedef struct pwm_driver_t pwm_driver_t;

// PWM 驱动操作接口 (虚函数表)
typedef struct {
  void (*start)(pwm_driver_t *self);
  void (*stop)(pwm_driver_t *self);
  void (*set_duty)(pwm_driver_t *self, uint32_t duty);
  void (*set_freq)(pwm_driver_t *self, uint32_t freq);
} pwm_driver_ops_t;

// PWM 驱动基类
struct pwm_driver_t {
  const pwm_driver_ops_t *ops;
};

// 辅助宏，方便调用
#define PWM_START(driver) (driver)->ops->start(driver)
#define PWM_STOP(driver) (driver)->ops->stop(driver)
#define PWM_SET_DUTY(driver, d) (driver)->ops->set_duty(driver, d)
#define PWM_SET_FREQ(driver, f) (driver)->ops->set_freq(driver, f)

#endif /* BSP_DEVICE_DRIVER_PWM_LED_PWM_DRIVER_H_ */
