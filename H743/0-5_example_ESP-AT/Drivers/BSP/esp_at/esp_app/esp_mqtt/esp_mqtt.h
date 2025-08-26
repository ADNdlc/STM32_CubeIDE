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

typedef void (*mqtt_event_cb_t)(mqtt_state_typedef new_state);


/* ============================ 公开API实现 ============================ */
void MQTT_init(mqtt_event_cb_t event_callback);

/* =================== MQTT服务器连接(根据AT命令定义) ====================== */
/*	@brief			连接onenet的一个对应设备,模块支支持一个0号连接
 *	@param client_id 	MQTT客户端ID,	对于云平台 即<设备名称/ID>, DeviceID
 *	@param username 	用户名,用于登陆	对于云平台 即<产品ID>
 *	@param password 	密码(暂时手动输入)
 */
void MQTT_connect(const char* client_id, const char* username, const char* password);

/*	@brief	断开 MQTT 连接，释放资源
 */
void MQTT_disconnect(void);

/*	@brief	获取MQTT服务器连接状态
 *	@return 连接状态
 */
mqtt_state_typedef MQTT_get_state(void);

/* ================== MQTT主题订阅与推送(模块通用api) ================= */
/*	@brief			订阅一个mqtt主题
 *	@param topic 	主题名称
 *	@param qos 		服务质量
 */
void MQTT_subscribe(const char* topic, int qos);

/*	@brief			向一个主题推送字符串消息
 *	@param topic 	主题名称
 *	@param data 	推送内容
 *	@param qos 		服务质量 0/1/2
 *	@param retain 	保留会话? 0/1
 */
void MQTT_publish(const char* topic, const char* data, uint8_t qos, uint8_t retain);

/* ================ 设备主题订阅与推送(onenet云功能) ================ */
/*	@brief		设置本设备名称(标识符),测试:test2
 *	@param c 	设备名称
 *	@return		执行结果
 */
uint8_t MQTT_Set_DeviceID(char *c);

/*	@brief		获取本设备名称(标识符)
 *	@return		设备名称
 */
char *MQTT_Get_DeviceID(void);

/*	@brief			推送传感器数据
 *	@param Module 	传感器对象
 */
void MQTT_publish_Module_data(const Module* Module);



/* ======================== MQTT URC 处理 ========================== */
//MQTT服务器连接状态 +MQTTCONNECTED
void MQTT_handle_urc_connected(const char* line);

//MQTT服务器断开状态 +MQTTDISCONNECTED
void MQTT_handle_urc_disconnected(const char* line);

/*	@brief	订阅信息回调  +MQTTSUBRECV:
 * 			找到set主题后的Json载荷并交给"云命令分发器"
 *	@param line 收到的内容
 */
void MQTT_handle_urc_recv(const char* line);


/*	@brief	用于回复接收到的云端命令
 *	@param id	云命令的消息id
 *	@param code	事件代码 成功:200
 *	@param msg	回复内容,随意
 */
void MQTT_send_reply(const char* id, int code, const char* msg);


#endif /* BSP_ESP_AT_ESP_APP_ESP_MQTT_ESP_MQTT_H_ */
