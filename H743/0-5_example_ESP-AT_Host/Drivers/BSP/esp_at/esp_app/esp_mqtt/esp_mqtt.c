/*
 * esp_mqtt.c
 *
 *  Created on: Aug 13, 2025
 *      Author: 12114
 */

#include "esp_mqtt.h"
#include "../../at_controller.h"
#include "../esp_wifi/esp_wifi.h"
#include <stdio.h>
#include <string.h>
#if USE_MY_MALLOC
#include "malloc/malloc.h"
#endif

#define longPWD 1	//长输入时,不知为何模块没有发送">",只有发送一次密码才能收到，但再发一遍也没有设置正确

// --- 私有状态和缓冲区 ---
static char DeviceID[64] = "test2";		//设备ID, 即Client ID

static mqtt_state_typedef g_mqtt_state = MQTT_STATE_NOUSERCFG;
static mqtt_command_cb_t g_cmd_cb = NULL;

#if !SAVE_CMD	//全局静态数据
static char usercfg_cmd_buf[256];
#if longPWD
static char longpwd_cmd_buf[64];
#endif
static char conn_cmd_buf[128];
static char sub_cmd_buf[128];
static char pub_cmd_buf[128]; 	// 这个只存命令本身
static char pub_payload_buffer[512];// 用于发布数据的 payload 缓冲区
// --- 私有命令对象 ---
static AT_Cmd_t cmd_mqtt_usercfg;		// 设置用户信息命令
#if longPWD
static AT_Cmd_t cmd_mqtt_long_password;	// 密码设置命令
#endif
static AT_Cmd_t cmd_mqtt_conn;			// MQTT Broker连接命令
static AT_Cmd_t cmd_mqtt_sub;			// 订阅命令
static AT_Cmd_t cmd_mqtt_pub;			// 推送命令
static AT_Cmd_t cmd_mqtt_clean;			// 清除设置命令(如果未启用自动重连，重连需清理后从头设置)
#endif

void MQTTBroker_connect(void);
// =========== 命令结果回调函数 ============
static void _set_userCFG_rsp_cb(AT_CmdResult_t result, const char* line) {
    if (result != AT_CMD_OK) {
        // 如果序列中的任何一步失败，重置状态
        g_mqtt_state = MQTT_STATE_NOUSERCFG;return;
    }
    if(result == AT_CMD_OK){
#if longPWD
    	g_mqtt_state = MQTT_STATE_NOPWD;	//进入下一项参数设置
#else
    	g_mqtt_state = MQTT_STATE_WAITCONNECT;//连接参数设置完成
    	MQTTBroker_connect();
#endif
    }
}
#if longPWD
static void _set_longPWD_rsp_cb(AT_CmdResult_t result, const char* line) {
    if (result != AT_CMD_OK) {
        // 如果序列中的任何一步失败，重置状态
        g_mqtt_state = MQTT_STATE_NOUSERCFG;return;
    }
    if((result == AT_CMD_OK)&&(g_mqtt_state == MQTT_STATE_NOPWD)){
    	g_mqtt_state = MQTT_STATE_WAITCONNECT;//连接参数设置完成
    	MQTTBroker_connect();
    }
}
#endif

static void _simple_mqtt_rsp_cb(AT_CmdResult_t result, const char* line) {
    if (result != AT_CMD_OK) {

    }
}

// mqtt连接命令的最终回调
static void _connect_rsp_cb(AT_CmdResult_t result, const char* line) {
    if (result == AT_CMD_OK) {
        // 命令被接受，等待 +MQTTCONNECTED URC
#ifndef NDEBUG
    printf("MQTT Connected!\r\n");
#endif
    } else {
        g_mqtt_state = MQTT_STATE_NOUSERCFG;
    }
}



/* ==================== 公开API实现 ==================== */

void MQTT_init(mqtt_command_cb_t cmd_callback) {
    g_cmd_cb = cmd_callback;
#if !SAVE_CMD
    // 初始化所有命令对象，让它们的 cmd_str 指向各自独立的缓冲区
    cmd_mqtt_usercfg = (AT_Cmd_t){
    	.cmd_str = usercfg_cmd_buf,
    	.timeout_ms = 5000,
		.response_cb = _set_userCFG_rsp_cb
    };
#if longPWD
    cmd_mqtt_long_password = (AT_Cmd_t){
    	.timeout_ms = 5000,
    	.response_cb = _set_longPWD_rsp_cb
    }; // data_to_send 在运行时设置
#endif
    cmd_mqtt_conn = (AT_Cmd_t){
    	.cmd_str = conn_cmd_buf,
    	.timeout_ms = 20000,
		.response_cb = _connect_rsp_cb
    };
    cmd_mqtt_sub = (AT_Cmd_t){
    	.cmd_str = sub_cmd_buf,
    	.timeout_ms = 5000,
		.response_cb = _simple_mqtt_rsp_cb
    };
    cmd_mqtt_pub = (AT_Cmd_t){
    	.cmd_str = pub_cmd_buf,
    	.data_to_send = pub_payload_buffer,
		.timeout_ms = 10000,
		.response_cb = _simple_mqtt_rsp_cb
    };
    cmd_mqtt_clean = (AT_Cmd_t){
    	.cmd_str = "AT+MQTTCLEAN=0\r\n",
    	.timeout_ms = 2000,
		.response_cb = _simple_mqtt_rsp_cb
    };
#endif
}


/* ========================================= MQTT服务器连接(根据AT命令定义) ========================================== */
/*	@brief			连接onenet的一个对应设备,模块支支持一个0号连接
 *
 *	@param client_id 	MQTT客户端ID,	对于云平台 即<设备名称/ID>, DeviceID
 *	@param username 	用户名,用于登陆	对于云平台 即<产品ID>
 *
 *	@param password 	密码(暂时手动输入)
 *
 */
void MQTT_connect(const char* client_id, const char* username, const char* password) {
	if(WiFi_get_state() != WIFI_STATE_GOT_IP){
#ifndef NDEBUG
    printf("MQTT_connect: NO Wifi\r\n");
#endif
		return;
	}
#ifndef NDEBUG
    printf("MQTT SetUserCFG...\r\n");
#endif
#if SAVE_CMD
#if USE_MY_MALLOC
	char* usercfg_cmd_buf = mymalloc(SRAMDTCM,256);
#if longPWD
	char* longpwd_cmd_buf = mymalloc(SRAMDTCM,64);
#endif
#else
    char usercfg_cmd_buf[128];
#if longPWD
    char longpwd_cmd_buf[64];
#endif
#endif
    AT_Cmd_t cmd_mqtt_usercfg = (AT_Cmd_t){
       	.cmd_str = usercfg_cmd_buf,
       	.timeout_ms = 5000,
   		.response_cb = _set_userCFG_rsp_cb
       };
#if longPWD
    AT_Cmd_t cmd_mqtt_long_password = (AT_Cmd_t){
       	.timeout_ms = 10000,
       	.response_cb = _set_longPWD_rsp_cb
       }; // data_to_send 在运行时设置
#endif
#endif
    // === OneNet连接序列 ===
    // 设置用户配置, 1: MQTT over TCP (无证书)

#if !longPWD
    snprintf(usercfg_cmd_buf, 256,
             "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\r\n", client_id, username, password);
    AT_controller_cmd_submit(&cmd_mqtt_usercfg);

#else
    snprintf(usercfg_cmd_buf, 256,
             "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"\",0,0,\"\r\n", client_id, username);
    AT_controller_cmd_submit(&cmd_mqtt_usercfg);
     //设置长密码, 写入longpwd_cmd_buf(数据在">"回调中发送)
    snprintf(longpwd_cmd_buf, 64,
             "AT+MQTTLONGPASSWORD=0,%d\r\n", strlen(password));
    // 动态设置命令对象的字段
    cmd_mqtt_long_password.cmd_str = longpwd_cmd_buf;
    cmd_mqtt_long_password.data_to_send = password;

    printf("PWD:%s\r\n", password);
    AT_controller_cmd_submit(&cmd_mqtt_long_password);
#endif
    // 连接Broker
    // 在long_password成功回调中提交
}

void MQTTBroker_connect(void){
#ifndef NDEBUG
    printf("MQTT Connecting...\r\n");
#endif
#if SAVE_CMD
#if USE_MY_MALLOC
	char* conn_cmd_buf = mymalloc(SRAMDTCM,128);
#else
    char conn_cmd_buf[128];
#endif
    AT_Cmd_t cmd_mqtt_conn = (AT_Cmd_t){
       	.cmd_str = conn_cmd_buf,
       	.timeout_ms = 20000,
   		.response_cb = _connect_rsp_cb
       };
#endif
    // 连接Broker, 写入conn_cmd_buf
    snprintf(conn_cmd_buf, 128,
             "AT+MQTTCONN=0,\"%s\",%d,0\r\n", MQTT_HOST, MQTT_PORT);
    AT_controller_cmd_submit(&cmd_mqtt_conn);

}
/*	@brief	断开 MQTT 连接，释放资源
 *			MQTT 设置不自动重连。如果 MQTT 建立连接后又断开，
 *			需要先发送 AT+MQTTCLEAN=0 命令清理信息，重新配置参数，再建立新的连接。
 */
void MQTT_disconnect(void) {
	AT_Cmd_t cmd_mqtt_clean = (AT_Cmd_t){
    	.cmd_str = "AT+MQTTCLEAN=0\r\n",
    	.timeout_ms = 2000,
		.response_cb = _simple_mqtt_rsp_cb
    };
    AT_controller_cmd_submit(&cmd_mqtt_clean);
}

/*	@brief	获取MQTT服务器连接状态
 *	@return 连接状态
 */
mqtt_state_typedef MQTT_get_state(void) {
    return g_mqtt_state;
}

/* ========================================= MQTT主题订阅与推送(模块通用api) ========================================== */
/*	@brief			订阅一个mqtt主题
 *
 *	@param topic 	主题名称
 *	@param qos 		服务质量
 *
 */
void MQTT_subscribe(const char* topic, int qos) {
	if(g_mqtt_state != MQTT_STATE_CONNECTED){
#ifndef NDEBUG
		printf("MQTT_subscribe: MQTT NOCONNECTED!");
#endif
		return;
	}
#if SAVE_CMD
#if USE_MY_MALLOC
	char* sub_cmd_buf = mymalloc(SRAMDTCM,128);
#else
	static char sub_cmd_buf[128];
#endif
    AT_Cmd_t cmd_mqtt_sub = (AT_Cmd_t){
    	.cmd_str = sub_cmd_buf,
    	.timeout_ms = 5000,
		.response_cb = _simple_mqtt_rsp_cb
    };
#endif
    snprintf(sub_cmd_buf, 128, "AT+MQTTSUB=0,\"%s\",%d\r\n", topic, qos);
    AT_controller_cmd_submit(&cmd_mqtt_sub);
}

/*	@brief			向一个主题推送字符串消息
 *
 *	@param topic 	主题名称
 *	@param data 	推送内容
 *	@param qos 		服务质量 0/1/2
 *	@param retain 	保留会话? 0/1
 *
 */
void MQTT_publish(const char* topic, const char* data, uint8_t qos, uint8_t retain){
    if (g_mqtt_state != MQTT_STATE_CONNECTED) {
#ifndef NDEBUG
    	printf("MQTT_publish: NOconnect!");
#endif
    	return;
    }
#if SAVE_CMD
#if USE_MY_MALLOC
	char* pub_cmd_buf = mymalloc(SRAMDTCM,128);
	char* pub_payload_buffer = mymalloc(SRAMDTCM,512);
#else
	char pub_cmd_buf[128]; 	// 这个只存命令本身
	char pub_payload_buffer[512];// 用于发布数据的 payload 缓冲区
#endif
    AT_Cmd_t cmd_mqtt_pub = (AT_Cmd_t){
    	.cmd_str = pub_cmd_buf,
    	.data_to_send = pub_payload_buffer,
		.timeout_ms = 10000,
		.response_cb = _simple_mqtt_rsp_cb
    };
#endif
    // 构建 AT+MQTTPUBRAW 命令, 写入独立的 pub_cmd_buf
	snprintf(pub_cmd_buf, 128,
			"AT+MQTTPUBRAW=0,\"%s\",%d,%d,%d\r\n", topic, strlen(data), qos, retain);
	AT_controller_cmd_submit(&cmd_mqtt_pub);//提交命令,data在回调中发送
}

/* ====================================== 设备主题订阅与推送(onenet云功能) ======================================= */
/*	@brief		设置本设备名称(标识符)
 *
 *	@param c 	设备名称
 *	@return		执行结果
 *
 */
uint8_t MQTT_Set_DeviceID(char *c){
	if(strlen(c) > sizeof(DeviceID)){
#ifndef NDEBUG
    	printf("DeviceID: too long!");
#endif
		return 1;
	}
	strcpy(DeviceID, c);
	return 0;
}

char *MQTT_Get_DeviceID(){
	return DeviceID;
}

///*	@brief			推送传感器数据
// *
// *	@param topic 	主题名称
// *	@param sensor 	传感器对象
// *	@param qos 		服务质量 0/1/2
// *	@param retain 	保留会话? 0/1
// *
// */
//void MQTT_publish_sensor_data(const Sensor* sensor) {
//    if (g_mqtt_state != MQTT_STATE_CONNECTED) {
//#ifndef NDEBUG
//    	printf("MQTT_publish: NOconnect!");
//#endif
//    	return;
//    }
//    if (g_mqtt_state != MQTT_STATE_CONNECTED) {
//#ifndef NDEBUG
//    	printf("MQTT_publish: NOconnect!");
//#endif
//    	return;
//    }
//
//
//#if SAVE_CMD
//#if USE_MY_MALLOC
//	char* pub_cmd_buf = mymalloc(SRAMDTCM,128);
//	char* pub_payload_buffer = mymalloc(SRAMDTCM,512);
//#else
//	static char pub_cmd_buf[128]; 	// 这个只存命令本身
//	static char pub_payload_buffer[512];// 用于发布数据的 payload 缓冲区
//#endif
//    AT_Cmd_t cmd_mqtt_pub = (AT_Cmd_t){
//    	.cmd_str = pub_cmd_buf,
//    	.data_to_send = pub_payload_buffer,
//		.timeout_ms = 10000,
//		.response_cb = _simple_mqtt_rsp_cb
//    };
//#endif
//
//    // 获取 Topic
//    // $sys/{pid}/{device-name}/thing/property/post
//    // 注意: {pid} 和 {device-name} 应该作为参数传入或从配置中读取
//    // 这里为了简化，我们硬编码
//    const char* pid = "SQKg9n0Ii0"; // 从你的文档中提取
//    const char* dev_name = "temperatureAndHumidity";
//    char topic[128];
//    snprintf(topic, sizeof(topic), "$sys/%s/%s/thing/property/post", pid, dev_name);
//    // 序列化数据到 payload
//    int payload_len = Sensor_Data_to_json_string(sensor, pub_payload_buffer, sizeof(pub_payload_buffer));
//    if (payload_len < 0) {
//        printf("Error: JSON serialization failed!\r\n");
//        return;
//    }
//    // 构建 AT+MQTTPUBRAW 命令, 写入独立的 pub_cmd_buf
//     snprintf(pub_cmd_buf, sizeof(pub_cmd_buf),
//              "AT+MQTTPUBRAW=0,\"%s\",%d,0,0\r\n", topic, payload_len);
//    AT_controller_cmd_submit(&cmd_mqtt_pub);
//}


/* ========================================= MQTT URC 处理 ========================================== */

void MQTT_handle_urc_connected(const char* line) {
    g_mqtt_state = MQTT_STATE_CONNECTED;
    printf("MQTT Connected to broker.\r\n");
    // 可以在这里自动订阅主题
}

void MQTT_handle_urc_disconnected(const char* line) {
    g_mqtt_state = MQTT_STATE_NOUSERCFG;
    printf("MQTT Disconnected from broker.\r\n");
}

void MQTT_handle_urc_recv(const char* line) {
    // 格式: +MQTTSUBRECV:0,"<topic>",<len>,<payload>
    char topic[128];
    char payload[256];
    int len;

    // 简化的解析，实际项目中可能需要更健壮的解析器
    const char* topic_start = strchr(line, '"');
    if (!topic_start) return;
    const char* topic_end = strchr(topic_start + 1, '"');
    if (!topic_end) return;

    int topic_len = topic_end - (topic_start + 1);
    if (topic_len < sizeof(topic)) {
        strncpy(topic, topic_start + 1, topic_len);
        topic[topic_len] = '\0';
    }

    const char* len_start = topic_end + 2; // 跳过 ",
    len = atoi(len_start);

    const char* payload_start = strchr(len_start, ',') + 1;
    if (payload_start && len < sizeof(payload)) {
        strncpy(payload, payload_start, len);
        payload[len] = '\0';

        // 调用上层注册的回调函数
        if (g_cmd_cb) {
            g_cmd_cb(topic, payload);
        }
    }
}


