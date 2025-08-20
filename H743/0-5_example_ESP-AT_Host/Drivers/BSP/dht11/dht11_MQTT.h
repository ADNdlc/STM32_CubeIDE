/*
 * dht11_MQTT.h
 *
 *  Created on: Jun 1, 2025
 *      Author: 12114
 */

#ifndef DHT11_DHT11_MQTT_H_
#define DHT11_DHT11_MQTT_H_

#include "esp_at/esp_app/esp_mqtt/esp_mqtt.h"
#include "dht11.h"

#define dht11_name "\"123\""
#define dht11_version "\"1.0\""

/*数据点信息*/
#define	T_TAG	"\"currentTemperature\""
#define	T_Type	DATA_int

#define	H_TAG	"\"currenthumidity\""
#define	H_Type	DATA_int

extern Sensor* Sensor_dht11;

/*===================================================函数========================================================*/

void dht11_MQTTInit();
uint8_t dht11_MQTT_updataANDpublish(Sensor* S,uint8_t* data);


#endif /* DHT11_DHT11_MQTT_H_ */
