/*
 * esp_mqtt.c
 *
 *  Created on: Aug 13, 2025
 *      Author: 12114
 */

#define USE_MY_MALLOC	0
#if USE_MY_MALLOC
#include "malloc/malloc.h"
#endif
#include "esp_mqtt.h"
#include "../../at_controller.h"
#include <stdio.h>
#include <string.h>

// --- 私有状态和缓冲区 ---
static mqtt_state_typedef g_mqtt_state = MQTT_STATE_NOCONNECTCFG;
static mqtt_command_cb_t g_cmd_cb = NULL;

static char usercfg_cmd_buf[128];
static char longpwd_cmd_buf[64];
static char conn_cmd_buf[128];
static char sub_cmd_buf[128];
static char pub_cmd_buf[128]; // 这个只存命令本身
// 用于发布数据的 payload 缓冲区
static char pub_payload_buffer[512];

// --- 私有命令对象 ---
static AT_Cmd_t cmd_mqtt_usercfg;		// 设置用户信息命令
static AT_Cmd_t cmd_mqtt_long_password;	// 密码设置命令
static AT_Cmd_t cmd_mqtt_conn;			// MQTT Broker连接命令
static AT_Cmd_t cmd_mqtt_sub;			// 订阅命令
static AT_Cmd_t cmd_mqtt_pub;			// 推送命令
static AT_Cmd_t cmd_mqtt_clean;			//


// --- 内部回调函数 ---
static void _set_mqtt_rsp_cb(AT_CmdResult_t result, const char* line) {
    if (result != AT_CMD_OK) {
        // 如果序列中的任何一步失败，重置状态
        g_mqtt_state = MQTT_STATE_NOCONNECTCFG;
    }
}

static void _simple_mqtt_rsp_cb(AT_CmdResult_t result, const char* line) {
    if (result != AT_CMD_OK) {
        // 如果序列中的任何一步失败，重置状态
        g_mqtt_state = MQTT_STATE_NOCONNECTCFG;
    }
}

// 连接命令的最终回调
static void _connect_rsp_cb(AT_CmdResult_t result, const char* line) {
    if (result == AT_CMD_OK) {
        // 命令被接受，等待 +MQTTCONNECTED URC
    } else {
        g_mqtt_state = MQTT_STATE_DISCONNECTED;
    }
}



// --- 公开API实现 ---

void MQTT_init(mqtt_command_cb_t cmd_callback) {
    g_cmd_cb = cmd_callback;

    // 初始化所有命令对象，让它们的 cmd_str 指向各自独立的缓冲区
    cmd_mqtt_usercfg = (AT_Cmd_t){
    	.cmd_str = usercfg_cmd_buf,
    	.timeout_ms = 5000,
		.response_cb = _set_mqtt_rsp_cb
    };
    cmd_mqtt_long_password = (AT_Cmd_t){
    	.timeout_ms = 5000,
    	.response_cb = _set_mqtt_rsp_cb
    }; // data_to_send 在运行时设置
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
}


/* ========================================= MQTT服务器连接 ========================================== */
/*	@brief			连接onenet的一个设备,模块支支持一个0号连接
 *
 *	@param client_id 	MQTT客户端ID,	即<设备名称/ID>
 *	@param username 	用户名,用于登陆	即<产品ID>
 *
 *	@param password 	密码(暂时手动输入)
 *
 */
void MQTT_connect(const char* client_id, const char* username, const char* password) {
    printf("MQTT Connecting...\r\n");

    // === OneNet连接序列 ===
    // 设置用户配置, 1:
    snprintf(usercfg_cmd_buf, sizeof(usercfg_cmd_buf),
             "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"\",0,0,\"\r\n", client_id, username);
    AT_controller_cmd_submit(&cmd_mqtt_usercfg);

    // 设置长密码, 写入longpwd_cmd_buf(数据在">"回调中发送)
    snprintf(longpwd_cmd_buf, sizeof(longpwd_cmd_buf),
             "AT+MQTTLONGPASSWORD=0,%d\r\n", strlen(password));
    // 动态设置命令对象的字段
    cmd_mqtt_long_password.cmd_str = longpwd_cmd_buf;
    cmd_mqtt_long_password.data_to_send = password;
    AT_controller_cmd_submit(&cmd_mqtt_long_password);

    // 连接Broker, 写入conn_cmd_buf
    snprintf(conn_cmd_buf, sizeof(conn_cmd_buf),
             "AT+MQTTCONN=0,\"%s\",%d,0\r\n", MQTT_HOST, MQTT_PORT);
    AT_controller_cmd_submit(&cmd_mqtt_conn);
}


void MQTT_disconnect(void) {
    AT_controller_cmd_submit(&cmd_mqtt_clean);
}

mqtt_state_typedef MQTT_get_state(void) {
    return g_mqtt_state;
}

/* ========================================= MQTT主题订阅与推送 ========================================== */
/*	@brief			订阅一个mqtt主题
 *
 *	@param topic 	主题名称
 *	@param qos 		服务质量
 *
 */
void MQTT_subscribe(const char* topic, int qos) {
    // 写入独立的 sub_cmd_buf
    snprintf(sub_cmd_buf, sizeof(sub_cmd_buf), "AT+MQTTSUB=0,\"%s\",%d\r\n", topic, qos);
    AT_controller_cmd_submit(&cmd_mqtt_sub);
}

/*	@brief			向主题推送消息
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
    	return;}


}

void MQTT_publish_sensor_data(const Sensor* sensor) {
    if (g_mqtt_state != MQTT_STATE_CONNECTED) {
#ifndef NDEBUG
    	printf("MQTT_publish: NOconnect!");
#endif
    	return;}
    // 获取 Topic
    // $sys/{pid}/{device-name}/thing/property/post
    // 注意: {pid} 和 {device-name} 应该作为参数传入或从配置中读取
    // 这里为了简化，我们硬编码
    const char* pid = "SQKg9n0Ii0"; // 从你的文档中提取
    const char* dev_name = "temperatureAndHumidity";
    char topic[128];
    snprintf(topic, sizeof(topic), "$sys/%s/%s/thing/property/post", pid, dev_name);
    // 序列化数据到 payload
    int payload_len = Sensor_Data_to_json_string(sensor, pub_payload_buffer, sizeof(pub_payload_buffer));
    if (payload_len < 0) {
        printf("Error: JSON serialization failed!\r\n");
        return;
    }
    // 构建 AT+MQTTPUBRAW 命令, 写入独立的 pub_cmd_buf
     snprintf(pub_cmd_buf, sizeof(pub_cmd_buf),
              "AT+MQTTPUBRAW=0,\"%s\",%d,0,0\r\n", topic, payload_len);
    AT_controller_cmd_submit(&cmd_mqtt_pub);
}


// --- URC 处理 ---
void MQTT_handle_urc_connected(const char* line) {
    g_mqtt_state = MQTT_STATE_CONNECTED;
    printf("MQTT Connected to broker.\r\n");
    // 可以在这里自动订阅主题
}

void MQTT_handle_urc_disconnected(const char* line) {
    g_mqtt_state = MQTT_STATE_DISCONNECTED;
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


