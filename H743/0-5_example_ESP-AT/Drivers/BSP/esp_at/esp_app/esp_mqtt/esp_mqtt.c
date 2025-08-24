/*
 * esp_mqtt.c
 *
 *  Created on: Aug 13, 2025
 *      Author: 12114
 */

#include "esp_mqtt.h"
#include "../../at_parser.h"
#include "../../at_controller.h"
#include "../esp_wifi/esp_wifi.h"
#include "cloud_dispatcher.h"
#include <stdio.h>
#include <string.h>
#if USE_MY_MALLOC
#include "malloc/malloc.h"
#endif

// --- 私有状态和缓冲区 ---
static char DeviceID[64] = "test2";		//设备ID, 即Client ID

#define topic_length	 128
#define payload_length	 256
#define topic_set_length 128
#if USE_MY_MALLOC
		static char* topic = NULL;
		static char* payload = NULL;
		static char* set_topic_pattern = NULL;
#else
		static char topic[topic_length];
		static char payload[payload_length];
		static char set_topic_pattern[topic_set_length];
#endif

static mqtt_state_typedef g_mqtt_state = MQTT_STATE_NOUSERCFG;	//MQTT连接状态
static mqtt_event_cb_t g_event_cb = NULL;						//事件回调


static void MQTTBroker_connect(void);

/* ================= 命令结果回调函数 ================= */
//MQTT用户属性设置
static void _set_userCFG_rsp_cb(AT_CmdResult_t result, const char* line) {
    if (result != AT_CMD_OK) {
        // 如果序列中的任何一步失败,重置状态
    	if(g_mqtt_state == MQTT_STATE_CONNECTED) return;//已连接再重复发送连接会返回ERROR但不会断开
        g_mqtt_state = MQTT_STATE_NOUSERCFG;return;
    }
    if(result == AT_CMD_OK){
    	g_mqtt_state = MQTT_STATE_NOPWD;//进入下一项参数设置
    }
}

//MQTT用户长密码设置
static void _set_longPWD_rsp_cb(AT_CmdResult_t result, const char* line) {
    if (result != AT_CMD_OK) {
        // 如果序列中的任何一步失败,重置状态
        g_mqtt_state = MQTT_STATE_NOUSERCFG;return;
    }
    if((result == AT_CMD_OK)&&(g_mqtt_state == MQTT_STATE_NOPWD)){
    	g_mqtt_state = MQTT_STATE_WAITCONNECT;//连接参数设置完成
    	MQTTBroker_connect();
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
    	//失败需重新设置用户信息
        g_mqtt_state = MQTT_STATE_NOUSERCFG;
    }
}
// mqtt连接命令的最终回调
static void _disconnect_rsp_cb(AT_CmdResult_t result, const char* line) {
    if (result == AT_CMD_OK) {
        // 命令被接受，等待 +MQTTCONNECTED URC
#ifndef NDEBUG
    printf("MQTT Disconnected OK!\r\n");
#endif
    	g_mqtt_state = MQTT_STATE_NOUSERCFG;
    } else {

    }
}
//结果回调
static void _simple_mqtt_rsp_cb(AT_CmdResult_t result, const char* line) {
    if (result != AT_CMD_OK) {
    }
}
// mqtt订阅命令的最终回调
static void _subscribe_rsp_cb(AT_CmdResult_t result, const char* line){
    if (result == AT_CMD_OK) {
#ifndef NDEBUG
    printf("_subscribe_rsp: OK!\r\n");
#endif
    } else {
#ifndef NDEBUG
    printf("_subscribe_rsp: ERROR!!!\r\n");
#endif
    }
}
// mqtt推送命令的最终回调
static void _publish_rsp_cb(AT_CmdResult_t result, const char* line){
    if (result == AT_CMD_OK) {

    } else {

    }
}

/* ============================ 公开API实现 ============================ */
/*	MQTT相关初始化
 *
 */
void MQTT_init(mqtt_event_cb_t event_callback) {
	g_event_cb = event_callback;
#if USE_MY_MALLOC
	topic = mymalloc(SRAMDTCM,topic_length);
	payload = mymalloc(SRAMDTCM,payload_length);
	set_topic_pattern = mymalloc(SRAMDTCM,topic_set_length);
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
	else if(g_mqtt_state == MQTT_STATE_CONNECTED){
#ifndef NDEBUG
    printf("MQTT_connect: has been connected.\r\n");
#endif
    	return;
	}
#ifndef NDEBUG
    printf("MQTT_connect: SetUserCFG...\r\n");
#endif
#if USE_MY_MALLOC
	char* usercfg_cmd_buf = mymalloc(SRAMDTCM,256);
	char* longpwd_cmd_buf = mymalloc(SRAMDTCM,64);
#else
    char usercfg_cmd_buf[128];
    char longpwd_cmd_buf[64];
#endif
    AT_Cmd_t cmd_mqtt_usercfg = (AT_Cmd_t){
       	.cmd_str = usercfg_cmd_buf,
       	.timeout_ms = 2000,
   		.response_cb = _set_userCFG_rsp_cb
       };
    AT_Cmd_t cmd_mqtt_long_password = (AT_Cmd_t){
       	.timeout_ms = 3000,
       	.response_cb = _set_longPWD_rsp_cb
       }; // data_to_send 在运行时设置
    //设置前先发送清理函数,不然非首次连接就ERROR
    MQTT_disconnect();
    // 设置用户配置, 连接方式 1: MQTT over TCP (无证书)
    snprintf(usercfg_cmd_buf, 256,
             "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"\",0,0,\"\r\n", client_id, username);
    AT_controller_cmd_submit(&cmd_mqtt_usercfg);

     // ----- 设置长密码, 写入longpwd_cmd_buf(数据在">"回调中发送) -----
    snprintf(longpwd_cmd_buf, 64,
             "AT+MQTTLONGPASSWORD=0,%d\r\n", strlen(password));
    // 动态设置命令对象的字段
    cmd_mqtt_long_password.cmd_str = longpwd_cmd_buf;
    cmd_mqtt_long_password.data_to_send = password;
    AT_controller_cmd_submit(&cmd_mqtt_long_password);

#if USE_MY_MALLOC
	myfree(SRAMDTCM,usercfg_cmd_buf);
	myfree(SRAMDTCM,longpwd_cmd_buf);
#endif
    // 连接Broker
    // 在long_password成功回调中提交
}

//提交连接命令(回调中使用)
static void MQTTBroker_connect(void){
#ifndef NDEBUG
    printf("MQTT Connecting...\r\n");
#endif
#if USE_MY_MALLOC
	char* conn_cmd_buf = mymalloc(SRAMDTCM,128);
#else
    char conn_cmd_buf[128];
#endif
    AT_Cmd_t cmd_mqtt_conn = (AT_Cmd_t){
       	.cmd_str = conn_cmd_buf,
       	.timeout_ms = 15000,
   		.response_cb = _connect_rsp_cb
       };
    // 连接Broker
    snprintf(conn_cmd_buf, 128,"AT+MQTTCONN=0,\"%s\",%d,0\r\n", MQTT_HOST, MQTT_PORT);
    AT_controller_cmd_submit(&cmd_mqtt_conn);
#if USE_MY_MALLOC
	myfree(SRAMDTCM,conn_cmd_buf);
#endif
}

/*	@brief	断开 MQTT 连接，释放资源
 *			MQTT 设置不自动重连。如果 MQTT 建立连接后又断开，
 *			需要先发送 AT+MQTTCLEAN=0 命令清理信息，重新配置参数，再建立新的连接。
 */
void MQTT_disconnect(void) {
	AT_Cmd_t cmd_mqtt_clean = (AT_Cmd_t){
    	.cmd_str = "AT+MQTTCLEAN=0\r\n",
    	.timeout_ms = 2000,
		.response_cb = _disconnect_rsp_cb
    };
    AT_controller_cmd_submit(&cmd_mqtt_clean);
    g_event_cb(MQTT_STATE_NOUSERCFG);
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
		printf("MQTT_subscribe: MQTT NOconnect!");
#endif
		return;
	}
#if USE_MY_MALLOC
	char* sub_cmd_buf = mymalloc(SRAMDTCM,128);
#else
	char sub_cmd_buf[128];
#endif
    AT_Cmd_t cmd_mqtt_sub = (AT_Cmd_t){
    	.cmd_str = sub_cmd_buf,
    	.timeout_ms = 5000,
		.response_cb = _subscribe_rsp_cb
    };
    snprintf(sub_cmd_buf, 128, "AT+MQTTSUB=0,\"%s\",%d\r\n", topic, qos);
    AT_controller_cmd_submit(&cmd_mqtt_sub);
#if USE_MY_MALLOC
	myfree(SRAMDTCM,sub_cmd_buf);
#endif
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
		.response_cb = _publish_rsp_cb,
    };
    // 构建 AT+MQTTPUBRAW 命令, 写入独立的 pub_cmd_buf
	snprintf(pub_cmd_buf, 128,
			"AT+MQTTPUBRAW=0,\"%s\",%d,%d,%d\r\n", topic, strlen(data), qos, retain);
	AT_controller_cmd_submit(&cmd_mqtt_pub);//提交命令,data在回调中发送
#if USE_MY_MALLOC
	myfree(SRAMDTCM,pub_cmd_buf);
	myfree(SRAMDTCM,pub_payload_buffer);
#endif
}

/* ====================================== 设备主题订阅与推送(onenet云功能) ======================================= */

/*	@brief		设置本设备名称(标识符),测试:test2
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

/*	@brief		获取本设备名称(标识符)
 *	@return		设备名称
 */
char *MQTT_Get_DeviceID(void){
	return DeviceID;
}

/*	@brief			推送传感器数据
 *	@param Module 	传感器对象
 *
 */
void MQTT_publish_Module_data(const Module* Module) {
#define payload_size 512
	   if (g_mqtt_state != MQTT_STATE_CONNECTED) {
	#ifndef NDEBUG
	    	printf("MQTT_publish: NOconnect!");
	#endif
	    	return;
	    }
#if USE_MY_MALLOC
		char* pub_cmd_buf = mymalloc(SRAMDTCM,128);
		char* pub_payload_buffer = mymalloc(SRAMDTCM,payload_size);
		char* topic = mymalloc(SRAMDTCM,128);
#else
		char pub_cmd_buf[128]; 	// 这个只存命令本身
		char pub_payload_buffer[payload_size];// 用于发布数据的 载荷 缓冲区
		char topic[128];
#endif
	    AT_Cmd_t cmd_mqtt_pub = (AT_Cmd_t){
	    	.cmd_str = pub_cmd_buf,
	    	.data_to_send = pub_payload_buffer,
			.timeout_ms = 10000,
			.response_cb = _publish_rsp_cb,
	    };
	// 获取 Topic
	// $sys/{pid}/{device-name}/thing/property/post
	const char* dev_name = MQTT_Get_DeviceID();	//获取本机名称
	snprintf(topic, 128, "$sys/%s/%s/thing/property/post", Product_ID, dev_name);
	// 序列化数据到 payload
	int payload_len = Module_Data_to_json_string(Module, pub_payload_buffer, payload_size);
	if (payload_len < 0) {
		printf("Error: JSON serialization failed!\r\n");
		return;
	}
	// 构建 AT+MQTTPUBRAW 命令
	snprintf(pub_cmd_buf, 128,
			"AT+MQTTPUBRAW=0,\"%s\",%d,0,0\r\n", topic, payload_len);
	// 提交发布命令
	AT_controller_cmd_submit(&cmd_mqtt_pub);
#if USE_MY_MALLOC
    myfree(SRAMDTCM,pub_cmd_buf);
	myfree(SRAMDTCM,pub_payload_buffer);
	myfree(SRAMDTCM,topic);
#endif
}


/* ========================================= MQTT URC 处理 ========================================== */

//MQTT服务器连接状态 +MQTTCONNECTED
void MQTT_handle_urc_connected(const char* line) {
    g_mqtt_state = MQTT_STATE_CONNECTED;
#ifndef NDEBUG
    printf("MQTT Connected to broker.\r\n");
#endif
    g_event_cb(MQTT_STATE_CONNECTED);
    // 可以在这里自动订阅主题
    MQTT_subscribe("$sys/SQKg9n0Ii0/test2/thing/property/set",0);
}

//MQTT服务器断开状态 +MQTTDISCONNECTED
void MQTT_handle_urc_disconnected(const char* line) {
    g_mqtt_state = MQTT_STATE_NOUSERCFG;
    g_event_cb(MQTT_STATE_NOUSERCFG);
#ifndef NDEBUG
    printf("MQTT Disconnected from broker.\r\n");
#endif
}

/*	@brief	回复云属性设置回复函数
 *			用于回复接收到的云端命令
 *
 *	@param id	云命令的消息id
 *	@param code	事件代码 成功:200
 *	@param msg	回复内容,随意
 *
 */
void MQTT_send_reply(const char* id, int code, const char* msg) {
    if (g_mqtt_state != MQTT_STATE_CONNECTED) return;
#if USE_MY_MALLOC
		char* reply_topic_buf = mymalloc(SRAMDTCM,128);
		char* reply_payload_buf = mymalloc(SRAMDTCM,128);
		char* reply_cmd_buf = mymalloc(SRAMDTCM,128);
#else
	    char reply_topic_buf[128];
	    char reply_payload_buf[128];
	    char reply_cmd_buf[128];
#endif
    AT_Cmd_t cmd_mqtt_reply;
    // 回复set_reply主题
    // Topic: $sys/{pid}/{device-name}/thing/property/set_reply
    const char* dev_name = MQTT_Get_DeviceID();
    snprintf(reply_topic_buf, 128,
             "$sys/%s/%s/thing/property/set_reply", Product_ID, dev_name);
    // 构建 Payload
    int payload_len = snprintf(reply_payload_buf, 128,
                               "{\"id\":\"%s\",\"code\":%d,\"msg\":\"%s\"}",
                               id, code, msg);
    // 构建 AT 命令
    snprintf(reply_cmd_buf, 128,
             "AT+MQTTPUBRAW=0,\"%s\",%d,0,0\r\n", reply_topic_buf, payload_len);
    // 初始化并提交命令
    cmd_mqtt_reply = (AT_Cmd_t) {
        .cmd_str = reply_cmd_buf,
        .data_to_send = reply_payload_buf,
        .timeout_ms = 10000,
        .response_cb = _simple_mqtt_rsp_cb
    };
    AT_controller_cmd_submit(&cmd_mqtt_reply);
#if USE_MY_MALLOC
    myfree(SRAMDTCM,reply_cmd_buf);
	myfree(SRAMDTCM,reply_payload_buf);
	myfree(SRAMDTCM,reply_topic_buf);
#endif
}

/*	@brief	订阅信息回调  +MQTTSUBRECV:
 * 			找到set主题后的Json载荷并交给"云命令分发器"
 *
 *	@param line 收到的内容
 *
 */
void MQTT_handle_urc_recv(const char* line) {
    // 格式: +MQTTSUBRECV:0,"<topic>",<len>,<payload>
    int len;
    const char* topic_start = strchr(line, '"');
    if (!topic_start) return;
    const char* topic_end = strchr(topic_start + 1, '"');
    if (!topic_end) return;

    int topic_len = topic_end - (topic_start + 1);
    if (topic_len < topic_length) {
        strncpy(topic, topic_start + 1, topic_len);
        topic[topic_len] = '\0';
    }
    const char* len_start = topic_end + 2; // 跳过 ",
    len = atoi(len_start);
    const char* payload_start = strchr(len_start, ',') + 1;
    if (payload_start && len < payload_length) {
        strncpy(payload, payload_start, len);
        payload[len] = '\0';
        const char* payload_start = strchr(len_start, ',') + 1;
        if (payload_start) {
               // 不再需要本地的 payload 缓冲区，直接将指针传递
               // 我们需要找到订阅的主题是否是 set topic
               // $sys/{pid}/{device-name}/thing/property/set
               snprintf(set_topic_pattern, topic_set_length,
                        "$sys/%s/%s/thing/property/set", Product_ID, MQTT_Get_DeviceID());
               if (strcmp(topic, set_topic_pattern) == 0) {
                   // 是属性设置命令交给云命令分发器处理
                   Cloud_dispatcher_process_command(payload_start);
               }
               else { // 是其他订阅消息，例如 post_reply
#ifndef NDEBUG
        printf("MQTT_handle_urc_recv: NO handler '%s'\r\n",line);
#endif
               }
        }
    }
}


