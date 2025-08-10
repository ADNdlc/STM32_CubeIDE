/*
 * MQTT_AT.c
 *
 *  Created on: May 19, 2025
 *      Author: ADMDDLC
 */

#ifndef ESP32_AT_MQTT_AT_C_
#define ESP32_AT_MQTT_AT_C_

#include "MQTT_AT.h"
#include "string.h"
#include <stdarg.h>


extern	DMA_HandleTypeDef	hdma_usart3_rx;

AT_ESP32_HandleTypeDef	ESP32_AT;
DMA_HandleTypeDef*	hdma_AT_rx	= &hdma_usart3_rx;


/*====================================================通信函数==========================================================*/


/* @brief	发送AT指令(非阻塞，状态由中断控制)
 * @param	Command 格式化字符串
 * @param	...     入参
 *
 */
void AT_Send(const char* __restrict__ Command, ...){
	//if(ESP32_AT.AT_state == Success){

		char buffer[1024];  //缓冲区大小(至少要装下网站的token)
		va_list args;

		va_start(args, Command);
		vsnprintf(buffer, sizeof(buffer), Command, args);
		va_end(args);

		uint16_t length = strlen(buffer);

		if (length + 2 < sizeof(buffer)) {
			buffer[length] = '\r';
			buffer[length + 1] = '\n';
			buffer[length + 2] = '\0';  // 更新字符串结束符
			length += 2;  // 更新长度
		} else {
			ESP32_AT.AT_state = SendERR;
			return;
		}
		//发送命令(非阻塞)
		HAL_UART_Transmit_DMA(ESP32_AT.Command_UART, (uint8_t*)buffer, length);
		ESP32_AT.AT_state = Sending;//状态置位
	//}
	//else{
	//	ESP32_AT.AT_state = SendERR;
	//}
}

/*====================================================中断相关==========================================================*/

/*	ESP32发送完成处理,写在 HAL_UART_TxCpltCallback 发送完成回调 函数里
 *
 */
void ESP32_TxCpltHandle(UART_HandleTypeDef *huart){
	if(huart->Instance == ESP32_AT.Command_UART->Instance){

		ESP32_AT.AT_state = Waiting;//发送完成等待模块响应
	}
}



/* @brief	ESP32接收完成回调,写在 HAL_UARTEx_RxEventCallback 串口接收事件 中断函数里
 * 			当模块回复完一次消息就会进入此函数，此时消息存在ESP32_AT.Reply_Data中，在此调用消息处理
 *
 * @param	huart 传入中断的参数
 * @param	Size  传入中断的参数
 *
 */
void ESP32_RxCpltHandle(UART_HandleTypeDef *huart,uint16_t Size){
	//这里是接收中断内
	if(huart->Instance == ESP32_AT.Command_UART->Instance){

//		if(ESP32_AT.AT_state == Success || ESP32_AT.AT_state == Sending){
//			//丢弃
//		}
//		else if(ESP32_AT.AT_state == Waiting){
//			ESP32_AT.AT_state = InHand;//处理中
//		}
//		else{
//			ESP32_AT.AT_state = Overload;
//			printf("Overload");
//		}

		HAL_UARTEx_ReceiveToIdle_DMA(ESP32_AT.Command_UART,(uint8_t*)ESP32_AT.Reply_Data,buff_Size);//开启接收，末参数为最大长度
		__HAL_DMA_DISABLE_IT(hdma_AT_rx,DMA_IT_HT);//关闭相关DMA接收过半中断


		ESP32_AT.writeIndex = Size;//表示这次接收了多少数据

#if(ATEtoUART1 == 1)
		printf("\r\nRx:%s\r\n",ESP32_AT.Reply_Data);//将刚收到的发送到串口一
		printf("\r\nState:%d\r\n",ESP32_AT.AT_state);
#endif
	}
}


/*==================================================消息处理=========================================================*/

/* @brief	消息处理
 * @param	buffer 数据地址
 * @param	Size   本次接收到的数据长度
 *
 */
void AT_Message(char* buffer){

}


/*====================================================功能函数==========================================================*/

/* @brief	模块功能初始化和基本工作模式加载与发送
 * @param	huartx 	: 通信端口 传入句柄地址 &huart3
 *
 */
void AT_ESP32_Init(UART_HandleTypeDef *huartx){
	//绑定通讯口
	ESP32_AT.Command_UART = huartx;

	ESP32_AT.readIndex = 0;//读写指针归位
	ESP32_AT.writeIndex = 0;

	ESP32_AT.AT_state = Success;

	//设置模式
	ESP32_AT.AT_WiFi.WiFi_MODE	= WIFI_MODE;

	//加载数据
	switch(ESP32_AT.AT_WiFi.WiFi_MODE){
	case WiFi_Mixed:
		ESP32_AT.AT_WiFi.Station_Data.Wifi_ssid = Wifi_SSID;
		ESP32_AT.AT_WiFi.Station_Data.wifi_pwd  = Wifi_PWD;
		ESP32_AT.AT_WiFi.AP_Data.AP_ssid = AP_SSID;
		ESP32_AT.AT_WiFi.AP_Data.AP_pwd  = AP_PWD;
		break;

	case WiFi_Station:
		ESP32_AT.AT_WiFi.Station_Data.Wifi_ssid = Wifi_SSID;
		ESP32_AT.AT_WiFi.Station_Data.wifi_pwd  = Wifi_PWD;
		break;

	case WiFi_SoftAP:
		ESP32_AT.AT_WiFi.AP_Data.AP_ssid = AP_SSID;
		ESP32_AT.AT_WiFi.AP_Data.AP_pwd  = AP_PWD;
		break;

	case WiFi_RFClose:
		break;

	default:
		break;
	}

	//加载MQTT设置
	ESP32_AT.AT_MQTT.MQTT_Data.MQTT_LinkID		= LinkID;
	ESP32_AT.AT_MQTT.MQTT_Data.MQTT_client_id	= client_id;
	ESP32_AT.AT_MQTT.MQTT_Data.MQTT_scheme		= scheme;
	ESP32_AT.AT_MQTT.MQTT_Data.MQTT_username	= username;
	ESP32_AT.AT_MQTT.MQTT_Data.MQTT_password	= password;

	AT_Send("AT+RST");
	HAL_Delay(2000);

	AT_Send("ATE1");

	//发送设置数据
	AT_Send("AT+CWMODE=%d,1",ESP32_AT.AT_WiFi.WiFi_MODE);//设置WiFi工作模式
	ESP32_AT.AT_WiFi.WiFi_state = WiFi_Init;

	//开启接收
	HAL_UARTEx_ReceiveToIdle_DMA(ESP32_AT.Command_UART,(uint8_t*)ESP32_AT.Reply_Data,buff_Size);//开启接收，末参数为最大长度
	__HAL_DMA_DISABLE_IT(hdma_AT_rx,DMA_IT_HT);//关闭相关DMA接收过半中断
}

/* @brief	连接已存储在Uer_Data.h中的WiFi
 *
 */
void AT_WiFi_Connect(){
	if(ESP32_AT.AT_WiFi.WiFi_state == WiFi_Init){
		AT_Send("AT+CWJAP=%s,%s",ESP32_AT.AT_WiFi.Station_Data.Wifi_ssid,ESP32_AT.AT_WiFi.Station_Data.wifi_pwd);
	}
	ESP32_AT.AT_WiFi.WiFi_state = WiFi_connected;
}

/* @brief	断开WiFi
 *
 */
void AT_WiFi_DisConnect(){
	if(ESP32_AT.AT_WiFi.WiFi_state == WiFi_connected){
		AT_Send("AT+CWQAP");
	}
}

/*======================================MQTT函数=================================================*/

/* @brief	发送User_Data.h中存储的MQTT设置
 *
 */
void AT_MQTT_Set(){
	AT_Send("AT+MQTTUSERCFG=0,1,%s,%s,\"\",0,0,\"\"",client_id,username);//未设置密钥
	HAL_Delay(700);
	int16_t	PWDlength = strlen(ESP32_AT.AT_MQTT.MQTT_Data.MQTT_password);
	AT_Send("AT+MQTTLONGPASSWORD=0,%d",PWDlength);
	//while(ESP32_AT.AT_state != WaitForIn){}//等待模块回复 > 可输入
	HAL_Delay(500);
	AT_Send("%s",password);//发送时间较长
	HAL_Delay(500);
}


/* @brief	连接云平台
 * @param	host 	:服务器域名
 * 			port	:服务器端口	max=65535
 */
void AT_MQTT_Connect(char* host,uint16_t port){
	AT_Send("AT+MQTTCONN=0,%s,%d,0",host,port);
	//尝试重连？

}

/*	@brief	断开MQTT连接
 * 			目前只有0号连接
 */
void AT_MQTT_DisConnect(){
	AT_Send("AT+MQTTCLEAN=0");
}

/* @brief	订阅主题
 * @param	Topic :主题名
 * 			qos   :服务质量	0/1/2
 */
void AT_MQTT_Subscribe(char* Topic,uint8_t qos){
	AT_Send("AT+MQTTSUB=0,%s,%d",Topic,qos);

}

/* @brief	取消订阅主题
 * @param	Topic :主题名
 *
 */
void AT_MQTT_UN_Subscribe(char* Topic){
	AT_Send("AT+MQTTUNSUB=0,%s",Topic);
}

/* @brief	推送长消息
 * @param	Topic :推送主题
 * 			json  :推送内容(按照网站规定格式)
 * 			qos   :服务质量	0/1/2
 * 			retain:消息保留	0/1		默认不保留
 */
void AT_MQTT_Publish(char* Topic,char* json,uint8_t qos,uint8_t retain){
	uint16_t Infoength = strlen(Topic);
	AT_Send("AT+MQTTPUBRAW=0,%s,%d,%d,%d",Topic,Infoength,qos,retain);//发布长消息
	while(ESP32_AT.AT_state != WaitForIn){}//等待模块回复 > 可输入
	AT_Send("%s",json);
}

#endif /* ESP32_AT_MQTT_AT_C_ */
