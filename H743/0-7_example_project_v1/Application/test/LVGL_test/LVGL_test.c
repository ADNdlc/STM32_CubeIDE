/*
 * LVGL_test.c
 *
 *  Created on: Nov 28, 2025
 *      Author: 12114
 */

#include "gpio_factory.h"
#include "gpio_driver.h"

void lvgl_test(void) {
    // 获取GPIO驱动实例
    gpio_driver_t* led_red = gpio_driver_get(GPIO_LED_RED);
    gpio_driver_t* led_green = gpio_driver_get(GPIO_LED_GREEN);
    gpio_driver_t* button_user = gpio_driver_get(GPIO_BUTTON_USER);

    // 测试GPIO输出
    if (led_red) {
        GPIO_WRITE(led_red, 1);  // 点亮红灯
    }

    if (led_green) {
        GPIO_WRITE(led_green, 0);  // 关闭绿灯
    }

    // 测试GPIO输入
    if (button_user) {
        uint8_t button_state = GPIO_READ(button_user);
        // 可以在这里处理按钮状态
        if (button_state) {
            // 按钮被按下
        }
    }
}