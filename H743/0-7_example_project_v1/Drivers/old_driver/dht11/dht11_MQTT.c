/*
 * dht11_MQTT.c
 *
 *  Created on: Jun 1, 2025
 *      Author: 12114
 */

#include "dht11_MQTT.h"


static Sensor* Sensor_dht11 = NULL;//设备指针

void dht11_MQTTInit(){
	//创建设备添加数据点
	Sensor_dht11 = Sensor_Init(dht11_message,dht11_version);
	Sensor_Add_Point(Sensor_dht11,Sensor_Create_Point(T_TAG,T_Type));
	Sensor_Add_Point(Sensor_dht11,Sensor_Create_Point(H_TAG,H_Type));
}


void dht11_MQTT_updateANDpublish(void){
	if(!DHT11_ReadData()){
		//uint32_t unixtime = RTC_getunix();
		uint32_t unixtime = 1755761472;
		Sensor_Change_PointValue(Sensor_dht11, T_TAG, unixtime, T_Type, DHT11_GetTemperature());
		Sensor_Change_PointValue(Sensor_dht11, H_TAG, unixtime, H_Type, DHT11_GetHumidity());
	}
	MQTT_publish_sensor_data(Sensor_dht11);
}


