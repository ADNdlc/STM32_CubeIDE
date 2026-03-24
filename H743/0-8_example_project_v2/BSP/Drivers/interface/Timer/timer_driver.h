/*
 * timer_driver.h
 *
 *  Created on: Feb 20, 2026
 *      Author: Antigravity
 */

#ifndef BSP_DEVICE_DRIVER_INTERFACE_TIMER_DRIVER_H_
#define BSP_DEVICE_DRIVER_INTERFACE_TIMER_DRIVER_H_

#include <stdbool.h>
#include <stdint.h>


// 前向声明
typedef struct timer_driver_t timer_driver_t;

// 回调函数原型
typedef void (*timer_callback_t)(void *context);

// 定时器驱动操作接口 (虚函数表)
typedef struct {
  int (*start)(timer_driver_t *self);
  int (*stop)(timer_driver_t *self);
  int (*set_period)(timer_driver_t *self, uint32_t period_ms);
  int (*set_callback)(timer_driver_t *self, timer_callback_t callback,
                      void *context);
} timer_driver_ops_t;

// 定时器驱动基类
struct timer_driver_t {
  const timer_driver_ops_t *ops;
};

// 辅助宏
#define TIMER_START(driver) (driver)->ops->start(driver)
#define TIMER_STOP(driver) (driver)->ops->stop(driver)
#define TIMER_SET_PERIOD(driver, ms) (driver)->ops->set_period(driver, ms)
#define TIMER_SET_CALLBACK(driver, cb, ctx)                                    \
  (driver)->ops->set_callback(driver, cb, ctx)

#endif /* BSP_DEVICE_DRIVER_INTERFACE_TIMER_DRIVER_H_ */
