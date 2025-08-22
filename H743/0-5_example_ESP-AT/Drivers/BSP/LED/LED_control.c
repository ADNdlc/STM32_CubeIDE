/*
 * LED_control.c
 *
 *  Created on: Aug 21, 2025
 *      Author: 12114
 */

#include "LED.h"
#include "esp_at/esp_app/esp_mqtt/cloud_dispatcher.h"

// 这是 "LED" 标识符对应的处理函数
static void handle_led_command(cJSON* value_item) {
    if (cJSON_IsBool(value_item)) {
        if (cJSON_IsTrue(value_item)) {
        	LED_ON
        } else {
        	LED_OFF
        }
    }
}

// 模块初始化函数
void led_control_init(void) {
    // 将自己的能力注册到云命令分发器
    Cloud_dispatcher_register_handler("LED", handle_led_command);
}


