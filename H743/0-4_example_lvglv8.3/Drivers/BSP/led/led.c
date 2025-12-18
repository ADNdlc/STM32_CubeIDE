/*
 * LED_control.c
 *
 *  Created on: Aug 21, 2025
 *      Author: 12114
 */

#include "LED.h"
#include <stdio.h>
#include <string.h>
#include "../service/device_manager.h"
#include "esp_at/esp_app/esp_mqtt/cloud_dispatcher.h"

LV_IMG_DECLARE(img_light);


static bool led_set_property_cb(device_data_t* device, const char* prop_id, property_value_t value) {
    for (int i = 0; i < device->property_count; i++) {
        if (strcmp(device->properties[i].id, prop_id) == 0) {
            // 找到属性，检查类型并设置值
            if (device->properties[i].type == PROP_TYPE_SWITCH) {
                device->properties[i].value.b = value.b;
                // 根据设备ID和属性ID控制实际的LED
                if (strcmp(device->deviceID, "Light1") == 0 && strcmp(prop_id, "led0") == 0) {
                    if (value.b) {
                        LED_ON(0);
                    } else {
                        LED_OFF(0);
                    }
                    printf("[Info]LED: Light1 power set to %s\r\n", value.b ? "ON" : "OFF");
                    return true;
                } else if (strcmp(device->deviceID, "Light2") == 0 && strcmp(prop_id, "led1") == 0) {
                    if (value.b) {
                        LED_ON(1);
                    } else {
                        LED_OFF(1);
                    }
                    printf("[Info]LED: Light2 power set to %s\r\n", value.b ? "ON" : "OFF");
                    return true;
                }
            }
        }
    }
    printf("[Error]LED: Property %s not found for device %s\r\n", prop_id, device->deviceID);
    return false;
}

static bool led_get_property_cb(device_data_t* device, const char* prop_id, property_value_t* value) {
    for (int i = 0; i < device->property_count; i++) {
        if (strcmp(device->properties[i].id, prop_id) == 0) {
            // 找到属性，返回其当前值
            *value = device->properties[i].value;
            return true;
        }
    }
    printf("[Error]LED: Property %s not found for device %s\r\n", prop_id, device->deviceID);
    return false;
}

void led_control_init(void){
    // 向云命令分发器注册MQTT处理器
    //Cloud_dispatcher_register_handler("LED", handle_cloud_led_command);

    device_data_t Light1 = {
        .deviceID = "Light1",
        .custom_name = "kitchen lights",
        .dev_img = &img_light,
        .property_count = 1,
        .set_property = led_set_property_cb,
        .get_property = led_get_property_cb,
        .user_data = NULL,
    };
    // 填充属性信息
    strcpy(Light1.properties[0].id, "led0");
    Light1.properties[0].type = PROP_TYPE_SWITCH;
    Light1.properties[0].value.b = false; // 默认关闭

    device_data_t Light2 = {
        .deviceID = "Light2",
        .custom_name = "bedroom lights",
        .dev_img = &img_light,
        .property_count = 1,
        .set_property = led_set_property_cb,
        .get_property = led_get_property_cb,
        .user_data = NULL,
    };
    // 填充属性信息
    strcpy(Light2.properties[0].id, "led1");
    Light2.properties[0].type = PROP_TYPE_SWITCH;
    Light2.properties[0].value.b = false; // 默认关闭

    // 将设备注册到设备管理器
    device_data_t* registered_device1 = DeviceManager_RegisterDevice(&Light1);
    device_data_t* registered_device2 = DeviceManager_RegisterDevice(&Light2);

    if (registered_device1 && registered_device2) {
        printf("[Info]LED: Device registered successfully\r\n");
    } else {
        printf("[Error]LED: Failed to register device\r\n");
    }
}

