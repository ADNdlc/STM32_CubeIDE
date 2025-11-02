/*
 * mqtt_service.h
 *
 *  Created on: Oct 31, 2025
 *      Author: 12114
 */

#ifndef BSP_ESP_AT_ESP_APP_ESP_MQTT_MQTT_SERVICE_H_
#define BSP_ESP_AT_ESP_APP_ESP_MQTT_MQTT_SERVICE_H_

// 初始化MQTT服务，包括订阅DeviceManager
void MqttService_Init(void);

// MQTT服务的周期性任务（例如定时上报传感器数据）
void MqttService_PeriodicTask(void);

#endif /* BSP_ESP_AT_ESP_APP_ESP_MQTT_MQTT_SERVICE_H_ */
