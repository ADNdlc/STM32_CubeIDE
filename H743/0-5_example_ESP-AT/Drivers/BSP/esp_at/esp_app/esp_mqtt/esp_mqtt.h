/*
 * esp_mqtt.h
 *
 *  Created on: Aug 13, 2025
 *      Author: 12114
 */

#ifndef BSP_ESP_AT_ESP_APP_ESP_MQTT_ESP_MQTT_H_
#define BSP_ESP_AT_ESP_APP_ESP_MQTT_ESP_MQTT_H_

#include "../../Module_Data.h" // 需要用到Module结构

#define  MQTT_HOST	"mqtts.heclouds.com"//onenet服务器地址
#define  MQTT_PORT	1883				//服务器端口
#define	 Product_ID	"SQKg9n0Ii0" // 此ID在onenet用于区分产品,即对应物模型。
								 //	此ID确定连接的是何种设备有何功能,不可更改
typedef enum {
	MQTT_STATE_NOUSERCFG,	//无用户信息
	MQTT_STATE_NOPWD,		//无密码
	//...
	MQTT_STATE_WAITCONNECT,	//已设置连接属性,未连接
    MQTT_STATE_CONNECTED,	//已连接 MQTT Broker
} mqtt_state_typedef;

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

// 使用Module对象发布物模型数据
void MQTT_publish_Module_data(const Module* Module);

void MQTT_connect(const char* client_id, const char* username, const char* password);
uint8_t MQTT_Set_DeviceID(char *c);
char *MQTT_Get_DeviceID();

/* ========================================= MQTT URC 处理 ========================================== */
void MQTT_handle_urc_connected(const char* line);//MQTT服务器连接状态
void MQTT_handle_urc_disconnected(const char* line);//MQTT服务器断开状态
void MQTT_handle_urc_recv(const char* line);	//收到订阅信息回调


#endif /* BSP_ESP_AT_ESP_APP_ESP_MQTT_ESP_MQTT_H_ */
