/*
 * dht11_MQTT.c
 *
 *  Created on: Jun 1, 2025
 *      Author: 12114
 */

#include "dht11_MQTT.h"
#include "esp_at/esp_app/RTC_SNTP/RTC_cal.h"

static Module* Module_dht11 = NULL;//设备指针

void dht11_MQTTInit(){
	//创建设备添加数据点
	Module_dht11 = Module_Init(dht11_message,dht11_version);
	Module_Add_Point(Module_dht11,Module_Create_Point(T_TAG,T_Type));
	Module_Add_Point(Module_dht11,Module_Create_Point(H_TAG,H_Type));
}

void dht11_MQTT_updateANDpublish(void){
	if(!DHT11_ReadData()){
		uint32_t unixtime = RTC_get_unix(8);
		Module_Change_PointValue(Module_dht11, T_TAG, unixtime, T_Type, DHT11_GetTemperature());
		Module_Change_PointValue(Module_dht11, H_TAG, unixtime, H_Type, DHT11_GetHumidity());
	}
	MQTT_publish_Module_data(Module_dht11);
}


