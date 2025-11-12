/*
 * LED_control.c
 *
 *  Created on: Aug 21, 2025
 *      Author: 12114
 */

#include "LED.h"
#include <stdio.h>
#include "../service/device_manager.h"
#include "esp_at/esp_app/esp_mqtt/cloud_dispatcher.h"

LV_IMG_DECLARE(img_light);


static bool led_set_property_cb(device_data_t* device, const char* prop_id, property_value_t value) {
    int temp;
    if (sscanf(device, "LED%d", &temp)) {
        switch(temp){
            case 1:
            	if(value)LED_SET(1,1);
            	else LED_SET(0,1);
            	break;
            case 2:
            	if(value)LED_SET(1,2);
            	else LED_SET(0,2);
            	break;
            default: return false;
        }
        return true; // 操作成功
    }
    else {
    	return false;
    }
}

static bool led_get_property_cb(device_data_t* device, const char* prop_id, property_value_t* value) {
    int temp;
    if (sscanf(device, "LED%d", &temp)) {

    }
    return false; // 不支持的属性
}

void led_control_init(void){
    // 向云命令分发器注册MQTT处理器
    //Cloud_dispatcher_register_handler("LED", handle_cloud_led_command);

    device_data_t Light1 = {
        .deviceID = "Light1",
        .custom_name = "kitchen lights",
        .dev_img = &img_light,
        .property_count = 2,
        .set_property = led_set_property_cb,
        .get_property = led_get_property_cb,
        .user_data = NULL,
    };
    // 填充属性信息
    strcpy(Light1.properties[0].id, "power1");
    Light1.properties[0].type = PROP_TYPE_SWITCH;
    Light1.properties[0].value.b = false; // 默认关闭

    device_data_t Light2 = {
        .deviceID = "Light2",
        .custom_name = "bedroom lights",
        .dev_img = &img_light,
        .property_count = 2,
        .set_property = led_set_property_cb,
        .get_property = led_get_property_cb,
        .user_data = NULL,
    };
    // 填充属性信息
    strcpy(Light2.properties[0].id, "power2");
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
