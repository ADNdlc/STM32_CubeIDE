/*
 * dht11_MQTT.c
 *
 *  Created on: Jun 1, 2025
 *      Author: 12114
 */

#include "dht11_MQTT.h"


Sensor* Sensor_dht11 = NULL;//设备指针


void dht11_MQTTInit(){
	//创建设备添加数据点
	Sensor_dht11 = Sensor_Init(dht11_name,dht11_version);
	Sensor_Add_Point(Sensor_dht11,Sensor_Create_Point(T_TAG,T_Type));
	Sensor_Add_Point(Sensor_dht11,Sensor_Create_Point(H_TAG,H_Type));
}



//uint8_t dht11_MQTT_updataANDpublish(Sensor* S,uint8_t* data){
//	char *JSON = NULL;
//
//	//更新时间
//	uint32_t Timestamp = cst_to_unix(&ESP_time);
//	//更新数据点值
//	Sensor_Change_PointValue(S,T_TAG,Timestamp,T_Type,data[1]);
//	Sensor_Change_PointValue(S,H_TAG,Timestamp,H_Type,data[0]);
//	JSON = MQTT_Bulid_JSON(S);
//
//	if(ESP32_MQTT.MQTT_state == MQTT_connected){
//		MQTT_Publish_Data(2,JSON,publish_Topic,0,0);//发布
//		return 0;
//	}
//	else{
//#if(ATEtoUART1 == 1)
//		printf("\r\nMQTT_Publish:NOconnected");
//#endif
//		return 1;
//	}
//}


