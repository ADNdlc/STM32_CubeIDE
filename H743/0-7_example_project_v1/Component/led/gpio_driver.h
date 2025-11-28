/*
 * gpio_driver.h
 *
 *  Created on: Nov 27, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_LED_GPIO_DRIVER_H_
#define BSP_DEVICE_DRIVER_LED_GPIO_DRIVER_H_

#include <stdint.h>

// 前向声明
typedef struct gpio_driver_t gpio_driver_t;

// GPIO 驱动操作接口 (虚函数表)
typedef struct {
    void (*gpio_write)(gpio_driver_t *self, uint8_t value);
    uint8_t (*gpio_read)(gpio_driver_t *self);
    void (*gpio_toggle)(gpio_driver_t *self);
} gpio_driver_ops_t;

// GPIO 驱动基类
struct gpio_driver_t {
    const gpio_driver_ops_t *ops;
};

// 辅助宏，方便调用
#define GPIO_WRITE(driver, value) (driver)->ops->gpio_write(driver, value)
#define GPIO_READ(driver) (driver)->ops->gpio_read(driver)
#define GPIO_TOGGLE(driver) (driver)->ops->gpio_toggle(driver)

#endif /* BSP_DEVICE_DRIVER_LED_GPIO_DRIVER_H_ */