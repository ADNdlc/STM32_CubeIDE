/*
 * key_test.c
 *
 *  Created on: Dec 5, 2025
 *      Author: 
 */
#include "all_tests_config.h"
#if led_test

#include "key_test.h"
#include "device_mapping.h"
#include "gpio_factory.h"
#include "gpio_key/gpio_key.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

// 按键事件回调函数
static void key_event_callback(gpio_key_t *key, KeyEvent event) {
    switch(event) {
        case KeyEvent_SinglePress:
            printf("Key Single Press Detected\r\n");
            break;
        case KeyEvent_DoublePress:
            printf("Key Double Press Detected\r\n");
            break;
        case KeyEvent_TriplePress:
            printf("Key Triple Press Detected\r\n");
            break;
        case KeyEvent_LongPress:
            printf("Key Long Press Detected\r\n");
            break;
        default:
            break;
    }
}

// GPIO按键测试用例
static void test_gpio_key(void) {
    // 获取按键驱动实例 (使用 GPIO_BUTTON_KEY0 作为测试对象)
    gpio_driver_t *driver = gpio_driver_get(GPIO_BUTTON_KEY0);

    if (driver) {
        // 创建按键对象 (低电平有效)
        gpio_key_t *key = Key_Create(driver, 0);
        
        if (key) {
            // 创建观察者并注册到按键
            static KeyObserver observer;
            observer.callback = key_event_callback;
            observer.next = NULL;
            
            Key_RegisterObserver(key, &observer);
            
            // 设置参数
            Key_SetDebounce(key, 20);       // 20ms去抖动
            Key_SetLongPress(key, 1000);    // 1000ms长按检测
            Key_SetClickTimeout(key, 300);  // 300ms点击超时
            
            printf("开始按键测试，请按下开发板上的KEY0按键...\r\n");
            
            // 模拟主循环中更新按键状态
            // 实际应用中应在主循环或定时器中调用Key_Update
            for(int i = 0; i < 5000; i++) {  // 运行约5秒用于测试
                Key_Update(key);
                HAL_Delay(1);
            }
            
            // 注销观察者
            Key_UnregisterObserver(key, &observer);
            
            // 销毁按键对象
            Key_Delete(key);
        }
    }
    
    printf("GPIO按键测试完成\r\n");
}

// 主测试入口
void key_test_run(void) {
    test_gpio_key();
}

#endif