/*
 * MQTT_AT.h
 *
 *  Created on: May 19, 2025
 *      Author: ADMDDLC
 */

#ifndef ESP32_AT_MQTT_AT_H_
#define ESP32_AT_MQTT_AT_H_

#include "../Printf/retarget.h"
#include "User_Data.h"
#include "usart.h"


#define	buff_Size	1024	//模块回复消息的缓冲区大小

typedef enum
{
	Success   =	0x00U,	//上一条命令执行成功结束(空闲)
	Waiting   =	0x01U,	//命令已发送等待相应
	WaitForIn =	0x02U,	//模块进入长输入 > 状态
	SendERR	  =	0x05U,
	Sending	  =	0x06U,
	InHand    = 0x07U,
	Overload  = 0x08

} AT_State;


/*===================================================WiFi========================================================*/

/*	<mode>：模式
	0: 无 Wi-Fi 模式，并且关闭 Wi-Fi RF
	1: Station 模式
	2: SoftAP 模式
	3: SoftAP+Station 模式
*/
typedef enum
{
	WiFi_RFClose =	0x00U,
	WiFi_Station =	0x01U,
	WiFi_SoftAP	 =	0x02U,
	WiFi_Mixed	 =	0x03U
} WiFi_Mode;


/*	WiFi状态
 */
typedef enum
{
	WiFi_connected	 =	0x00U,
	WiFi_Init		 =  0x01U,
	WiFi_connecting  =	0x02U,
	WiFi_connectFail =	0x03U,
	WiFiERR	 		 =	0x04U,
} WiFi_State;


/*	存储已保存的WiFi信息
 */
typedef struct
{
	char* Wifi_ssid;	//名
	char* wifi_pwd;		//密码

} Station_Data;


/*	存储自身的的WiFi信息
 */
typedef struct
{
	char* AP_ssid;	//名
	char* AP_pwd;	//密码

} AP_Data;


/*	WiFi功能
 *
 */
typedef struct
{
	WiFi_Mode			WiFi_MODE;		//当前的WiFi模式
	volatile WiFi_State	WiFi_state;		//当前的WiFi状态

	Station_Data		Station_Data;	//记录的WiFi信息
	AP_Data				AP_Data;


} AT_WiFi_HandleTypeDef;


/*===================================================MQTT========================================================*/

typedef enum
{
	MQTT_connected	 =	0x00U,
	MQTT_Init		 =  0x01U,
	MQTT_connecting  =	0x02U,
	MQTT_connectFail =	0x03U,
	MQTTERR	 		 =	0x04U,

} MQTT_State;


/*	存储MQTT用户属性
 */
typedef struct
{
	uint8_t		MQTT_LinkID;		//MQTT连接ID
	uint8_t		MQTT_scheme;		//连接方式
	char*		MQTT_client_id;		//网站的	“设备名称/ID”
	char*		MQTT_username;		//用户名，用于登陆 MQTT broker, 网站的"产品ID"
	char*		MQTT_password;		//密码，使用tokon工具生成

} MQTT_Data;


/*	MQTT功能
 *
 */
typedef struct
{
	volatile MQTT_State	MQTT_state;
	MQTT_Data			MQTT_Data;


} AT_MQTT_HandleTypeDef;


/*=============================================模块==========================================*/

typedef struct
{
	UART_HandleTypeDef*		Command_UART;		//传入串口外设句柄"地址"
	volatile AT_State		AT_state;

	AT_WiFi_HandleTypeDef	AT_WiFi;
	AT_MQTT_HandleTypeDef	AT_MQTT;

	uint16_t				writeIndex;			//写索引
	uint16_t				readIndex;			//读索引
	char 					Reply_Data[buff_Size];	//数据缓冲区
} AT_ESP32_HandleTypeDef;


/*=============================================外部声明==========================================*/
extern DMA_HandleTypeDef 		hdma_usart3_rx;//声明外部句柄
extern AT_ESP32_HandleTypeDef	ESP32_AT;

/*====================================================基本函数==========================================================*/

void AT_ESP32_Init(UART_HandleTypeDef *huartx);
void AT_Send(const char* __restrict__ Command, ...);

/*====================================================中断相关==========================================================*/

void ESP32_TxCpltHandle(UART_HandleTypeDef *huart);
void ESP32_RxCpltHandle(UART_HandleTypeDef *huart,uint16_t );

/*====================================================消息处理==========================================================*/

void AT_Message(char* buffer);

void Add_ReadIndex(uint8_t length);

char Read_buffer(uint8_t i);

uint8_t Get_UNhandled();

uint8_t Get_Empty();

uint8_t Write_buffer(char *data,uint16_t length);

/*====================================================WiFi函数==========================================================*/

void AT_WiFi_Connect();
void AT_WiFi_DisConnect();

/*======================================MQTT函数=================================================*/

void AT_MQTT_Set();
void AT_MQTT_Connect(char* host,uint16_t port);
void AT_MQTT_DisConnect();
void AT_MQTT_Subscribe(char* Topic,uint8_t qos);
void AT_MQTT_UN_Subscribe(char* Topic);
void AT_MQTT_Publish(char* Topic,char* json,uint8_t qos,uint8_t retain);


#endif /* ESP32_AT_MQTT_AT_H_ */
