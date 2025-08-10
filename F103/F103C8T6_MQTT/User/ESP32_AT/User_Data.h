/*
 * User_Data.h
 *
 *  Created on: May 19, 2025
 *      Author: ADMDDLC
 */

#ifndef ESP32_AT_USER_DATA_H_
#define ESP32_AT_USER_DATA_H_

//所有命令或配置信息均以ASCLL发送

/*工作模式*/
#define ATEtoUART1	1					//模块回复发送到printf
#define WIFI_MODE	WiFi_Mixed

/*WiFi信息*/
#define Wifi_SSID	"\"test2\""
#define Wifi_PWD  	"\"yu778834\""

#define AP_SSID		"\"ESP_32\""
#define AP_PWD  	"\"yu778866\""


//服务器地址和端口
#define MQTT_host	"\"mqtts.heclouds.com\""	//域名
#define MQTT_port	1883						//端口

//MQTT连接设置
#define	LinkID		0						//MQTT连接ID,目前只支持0
#define	scheme		1						//连接方式

#define	client_id	"\"temperatureAndHumidity\""	//网站的	“设备名称/ID”
#define	username	"\"SQKg9n0Ii0\""				//用户名，用于登陆 MQTT broker, 网站的"产品ID"
												//密码，使用tokon工具生成
#define	password	"\"version=2018-10-31&res=products%2FSQKg9n0Ii0%2Fdevices%2FtemperatureAndHumidity&et=1757458587&method=md5&sign=YCozJxz%2BPX0Qf1coXSUd0A%3D%3D\""

//MQTT主题和信息
					//这是信息回复主题
#define Info_Topic		"\"$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/post/reply\""
					//本地数据推送主题
#define publish_Topic	"\"$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/post\""

//传感器数据推送格式
#define Data_Info		"{\"id\":\"123\",\"version\":\"1.0\",\"params\":{\"currentTemperature\":{\"value\":22,\"time\":1747458287111},\"currenthumidity\":{\"value\":33,\"time\":1747458287111}}}"



#endif /* ESP32_AT_USER_DATA_H_ */
