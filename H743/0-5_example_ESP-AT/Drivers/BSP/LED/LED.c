/*
 * LED_control.c
 *
 *  Created on: Aug 21, 2025
 *      Author: 12114
 */

#include "LED.h"
#include "esp_at/esp_app/esp_mqtt/cloud_dispatcher.h"
#include "esp_at/esp_app/esp_mqtt/esp_mqtt.h"
#include "esp_at/esp_app/RTC_SNTP/RTC_cal.h"
#include "esp_at/Module_Data.h"

#define LED_ON		HAL_GPIO_WritePin(LED_Port,LED_Pin,GPIO_PIN_RESET)
#define LED_OFF		HAL_GPIO_WritePin(LED_Port,LED_Pin,GPIO_PIN_SET)
#define LED_SET(x)	(X)?LED_OFF:LED_ON
#define LED_READ	HAL_GPIO_ReadPin(LED_Port,LED_Pin)?false:true
#define LED_TOGGLE	HAL_GPIO_TogglePin(LED_Port,LED_Pin)

static Module* Module_LED = NULL;
/*数据点信息*/
#define	led_TAG		"LED"		//标识符
#define	led_Type	DATA_bool	//类型

// 云命令处理函数, LED
static void handle_led_command(cJSON* value_item) {
    if (cJSON_IsBool(value_item)) {
        if (cJSON_IsTrue(value_item)) {
        	LED_ON;
        } else {
        	LED_OFF;
        }
        LED_MQTT_publish();
    }
}

// 模块初始化函数
void LED_init(void) {
	//创建设备添加数据点
	Module_LED = Module_Init("22","1.0");
	Module_Add_Point(Module_LED,Module_Create_Point(led_TAG,led_Type));
    // 将自己的能力注册到云命令分发器
	Cloud_dispatcher_register("LED", handle_led_command);
}

void LED_MQTT_publish(void){
	uint32_t unixtime = RTC_get_unix(8);
	Module_Change_PointValue(Module_LED, led_TAG, unixtime, led_Type, LED_READ);
	MQTT_publish_Module_data(Module_LED);
}

uint8_t LED_Get_States(void){
	return LED_READ;
}

