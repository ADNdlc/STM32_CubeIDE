/*
 * dht11_MQTT.h
 *
 *  Created on: Jun 1, 2025
 *      Author: 12114
 */

#ifndef DHT11_DHT11_MQTT_H_
#define DHT11_DHT11_MQTT_H_

#include "esp_at/esp_app/esp_mqtt/esp_mqtt.h"
#include "esp_at/Sensor_Data.h"
#include "dht11.h"

#define dht11_message "11"
#define dht11_version "1.0"

/*数据点信息*/
#define	T_TAG	"currentTemperature"	//标识符
#define	T_Type	DATA_int					//类型

#define	H_TAG	"currenthumidity"		//标识符
#define	H_Type	DATA_int					//类型

void dht11_MQTTInit();
void dht11_MQTT_updateANDpublish(void);


#endif /* DHT11_DHT11_MQTT_H_ */
