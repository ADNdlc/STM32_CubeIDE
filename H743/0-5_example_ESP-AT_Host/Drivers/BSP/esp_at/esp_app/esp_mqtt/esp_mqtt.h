/*
 * esp_mqtt.h
 *
 *  Created on: Aug 13, 2025
 *      Author: 12114
 */

#ifndef BSP_ESP_AT_ESP_APP_ESP_MQTT_ESP_MQTT_H_
#define BSP_ESP_AT_ESP_APP_ESP_MQTT_ESP_MQTT_H_

#include "../../Sensor_Data.h" // 需要用到Sensor结构

#define  MQTT_HOST	"mqtts.heclouds.com"
#define  MQTT_PORT	1883

#if 0
typedef enum {
    MQTT_STATE_DISCONNECTED,
    MQTT_STATE_CONNECTING,
    MQTT_STATE_CONNECTED,
} mqtt_state_typedef;
#endif
#if 1
typedef enum {
	MQTT_STATE_NOCONNECTCFG,	//未初始化
    MQTT_STATE_DISCONNECTED,	//已设置连接属性,未连接
    MQTT_STATE_CONNECTED,		//已连接 MQTT Broker
} mqtt_state_typedef;
#endif

// 云端命令下发的回调函数指针
// topic 和 payload 指向临时缓冲区，如果需要长期保存，请在回调中拷贝
typedef void (*mqtt_command_cb_t)(const char* topic, const char* payload);

// --- 公开API ---

void MQTT_init(mqtt_command_cb_t cmd_callback);
void MQTT_connect(const char* client_id, const char* username, const char* password);
void MQTT_disconnect(void);
mqtt_state_typedef MQTT_get_state(void);

// 订阅一个主题
void MQTT_subscribe(const char* topic, int qos);

// 使用Sensor对象发布物模型数据
void MQTT_publish_sensor_data(const Sensor* sensor);


// --- 由Dispatcher调用的URC处理函数 ---
void MQTT_handle_urc_connected(const char* line);
void MQTT_handle_urc_disconnected(const char* line);
void MQTT_handle_urc_recv(const char* line);

#endif /* BSP_ESP_AT_ESP_APP_ESP_MQTT_ESP_MQTT_H_ */
