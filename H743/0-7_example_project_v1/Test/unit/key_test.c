/*
 * key_test.c
 *
 *  Created on: Dec 5, 2025
 *      Author: 
 */
#include "all_tests_config.h"
#if _key_test_

#include "key_test.h"
#include "device_mapping.h"
#include "gpio_factory.h"
#include "gpio_key/gpio_key.h"
#include "main.h"


//暂时使用led辅助测试
#include "gpio_led/gpio_led.h"
#include "sys.h"

gpio_led_t *led;

// 按键事件回调函数
static void key0_event_callback(gpio_key_t *key, KeyEvent event) {
    switch(event) {
        case KeyEvent_SinglePress:
            led_hal_on((led_hal_t *)led);
            break;
        case KeyEvent_DoublePress:
            led_hal_off((led_hal_t *)led);
            break;
        case KeyEvent_TriplePress:
            
            break;
        case KeyEvent_LongPress:
            
            break;
        default:
            break;
    }
}

// KEY1按键事件回调函数
static void key1_event_callback(gpio_key_t *key, KeyEvent event) {
    switch(event) {
        case KeyEvent_SinglePress:
            // 单击KEY1切换LED状态
            gpio_led_toggle((gpio_led_t *)led);
            break;
        case KeyEvent_DoublePress:
            // 双击KEY1使LED闪烁两次
            for (int i = 0; i < 2; i++) {
            	gpio_led_toggle(led);
                sys_delay_ms(400);
                gpio_led_toggle(led);
                sys_delay_ms(400);
            }
            break;
        case KeyEvent_TriplePress:
            // 三击KEY1使LED快速闪烁
            for (int i = 0; i < 5; i++) {
            	gpio_led_toggle(led);
                sys_delay_ms(150);
            	gpio_led_toggle(led);
                sys_delay_ms(150);
            }
            break;
        case KeyEvent_LongPress:
            // 长按KEY1使LED持续亮起
            led_hal_on((led_hal_t *)led);
            break;
        default:
            break;
    }
}

// KEY2按键事件回调函数
static void key2_event_callback(gpio_key_t *key, KeyEvent event) {
    switch(event) {
        case KeyEvent_SinglePress:
            // 单击KEY2使LED熄灭
            led_hal_off((led_hal_t *)led);
            break;
        case KeyEvent_DoublePress:
            // 双击KEY2使LED以较慢频率闪烁
            for (int i = 0; i < 3; i++) {
            	gpio_led_toggle(led);
                sys_delay_ms(500);
            	gpio_led_toggle(led);
                sys_delay_ms(500);
            }
            break;
        case KeyEvent_TriplePress:
            // 三击KEY2使LED以较快频率闪烁
            for (int i = 0; i < 10; i++) {
            	gpio_led_toggle(led);
                sys_delay_ms(100);
            	gpio_led_toggle(led);
                sys_delay_ms(100);
            }
            break;
        case KeyEvent_LongPress:
            // 长按KEY2使LED持续熄灭
            led_hal_off((led_hal_t *)led);
            break;
        default:
            break;
    }
}

// GPIO按键测试用例
static void test_gpio_key(void) {
    // 创建辅助 LED 对象
    led = gpio_led_create(gpio_driver_get(GPIO_LED_0), 0);
    led_hal_off((led_hal_t *)led);

    // 获取按键驱动实例 (使用 GPIO_BUTTON_KEY0 作为测试对象)
    gpio_driver_t *driver0 = gpio_driver_get(GPIO_BUTTON_KEY0);
    gpio_driver_t *driver1 = gpio_driver_get(GPIO_BUTTON_KEY1);
    gpio_driver_t *driver2 = gpio_driver_get(GPIO_BUTTON_KEY2);

    if (driver0 && driver1 && driver2) {
        // 创建按键对象 (低电平有效)
        gpio_key_t *key0 = Key_Create(driver0, 0);
        gpio_key_t *key1 = Key_Create(driver1, 0);
        gpio_key_t *key2 = Key_Create(driver2, 0);

        
        if (key0 && key1 && key2) {
            // 创建观察者并注册到按键
            static KeyObserver observer0, observer1, observer2;
            observer0.callback = key0_event_callback;
            observer0.next = NULL;
            
            observer1.callback = key1_event_callback;
            observer1.next = NULL;
            
            observer2.callback = key2_event_callback;
            observer2.next = NULL;
            
            Key_RegisterObserver(key0, &observer0);
            Key_RegisterObserver(key1, &observer1);
            Key_RegisterObserver(key2, &observer2);
            
            // 设置参数
            Key_SetDebounce(key0, 20);       // 20ms去抖动
            Key_SetLongPress(key0, 800);    // 长按检测
            Key_SetClickTimeout(key0, 300);  // 300ms点击超时
            
            Key_SetDebounce(key1, 20);       // 20ms去抖动
            Key_SetLongPress(key1, 800);    // 长按检测
            Key_SetClickTimeout(key1, 300);  // 300ms点击超时
            
            Key_SetDebounce(key2, 20);       // 20ms去抖动
            Key_SetLongPress(key2, 800);    // 长按检测
            Key_SetClickTimeout(key2, 300);  // 300ms点击超时
            
            
            // 模拟主循环中更新按键状态
            // 实际应用中应在主循环或定时器中调用Key_Update
            for(int i = 0; i < 10000; i++) {  // 运行约10秒用于测试
                Key_Update(key0);
                Key_Update(key1);
                Key_Update(key2);
                sys_delay_ms(1);
            }
            
            // 注销观察者
            Key_UnregisterObserver(key0, &observer0);
            Key_UnregisterObserver(key1, &observer1);
            Key_UnregisterObserver(key2, &observer2);
            
            // 销毁按键对象
            Key_Delete(key0);
            Key_Delete(key1);
            Key_Delete(key2);
        }
    }
    
}

// 主测试入口
void key_test_run(void) {
    test_gpio_key();
}

#endif
