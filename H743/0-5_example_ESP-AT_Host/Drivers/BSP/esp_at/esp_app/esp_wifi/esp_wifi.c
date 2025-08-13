/*
 * esp_wifi.c
 *
 *  Created on: Aug 13, 2025
 *      Author: 12114
 */


#include "esp_wifi.h"
#include <string.h>
#include <stdio.h>
#include "../../at_controller.h"

typedef enum {
	Closed = 0,
	Station,
	SoftAP,
	Mixed,
} wifi_mode_typedef;//模块工作模式


// --- 私有状态和回调 ---

static wifi_state_typedef g_wifi_state = WIFI_STATE_UNCONNECTED;//模块WiFi连接状态
static wifi_event_cb_t g_state_event_cb = NULL;					//状态改变时调用,由上层实现,可用来通知上层

// --- 私有命令对象和缓冲区 ---

// 用于AT+CWMODE=1
static AT_Cmd_t cmd_set_mode;
// 用于AT+CWJAP="ssid","pwd"
static AT_Cmd_t cmd_join_ap;
// 用于AT+CWQAP
static AT_Cmd_t cmd_quit_ap;

// 用于动态构建命令的缓冲区
static char wifi_info_cmd_buffer[64];


/**
 * 更新内部记录的模块wifi连接状态
 */
static void WiFi_update_state(wifi_state_typedef new_state) {
    if (g_wifi_state != new_state) {
        g_wifi_state = new_state;
#ifndef NDEBUG
        printf("WiFi state changed to: %d\r\n", new_state);
#endif
        // 如果上层注册了回调，就调用它
        if (g_state_event_cb) {
        	g_state_event_cb(new_state);
        }
    }
}

// 统一的简单响应回调
static void _simple_rsp_cb(AT_CmdResult_t result, const char* line) {
    if (result == AT_CMD_OK) {

    } else {

    }
}

// 连接AP命令的响应回调
static void _connect_rsp_cb(AT_CmdResult_t result, const char* line) {
    if (result == AT_CMD_OK) {
        // AT+CWJAP命令逐条返回  WIFI CONNECTED, WIFI GOT IP, OK

    } else {
        // 如果命令本身就失败了 (例如参数错误), 则连接失败
        WiFi_update_state(WIFI_STATE_DISCONNECTED);
    }
}

/* --- URC处理函数 和 对应响应头 --- */
// "WIFI CONNECTED"
void WiFi_handle_urc_connected(const char* line) {
    WiFi_update_state(WIFI_STATE_NO_IP);//按模块行为来
}

// "WIFI GOT IP"
void WiFi_handle_urc_got_ip(const char* line) {
    WiFi_update_state(WIFI_STATE_GOT_IP);//只有此函数被调用才真正连接成功
}

// "WIFI DISCONNECT"
void WiFi_handle_urc_disconnect(const char* line) {
    WiFi_update_state(WIFI_STATE_DISCONNECTED);
}



// --- 公开API实现 ---
/**
 * 初始化命令体,可注册状态更新回调
 */
void WiFi_init(wifi_event_cb_t event_callback) {
    WiFi_update_state(WIFI_STATE_UNCONNECTED);
    g_state_event_cb = event_callback;
    // 初始化所有命令对象
    cmd_set_mode = (AT_Cmd_t){
        .cmd_str = "AT+CWMODE=1,0\r\n",//station,禁止自动连接AP
        .timeout_ms = 1000,
        .response_cb = _simple_rsp_cb,
    };
    cmd_join_ap = (AT_Cmd_t){
        .cmd_str = wifi_info_cmd_buffer, // 指向动态缓冲区
        .timeout_ms = 10000, // 连接WiFi超时时间需要长一些
        .response_cb = _connect_rsp_cb,
    };
    cmd_quit_ap = (AT_Cmd_t){
        .cmd_str = "AT+CWQAP\r\n",
        .timeout_ms = 1000,
        .response_cb = _simple_rsp_cb,
    };
}

/**
 * 发送连接命令
 * 此函数不处理执行结果,它只负责提交命令
 */
void WiFi_connect(wifi_info_t* APdata) {
    // 动态构建AT+CWJAP命令
    snprintf(wifi_info_cmd_buffer, sizeof(wifi_info_cmd_buffer), "AT+CWJAP=\"%s\",\"%s\"\r\n", APdata->SSID, APdata->PWD);

    WiFi_update_state(WIFI_STATE_CONNECTING);// 更新状态为“正在连接”

    // 提交“设置模式”和“加入AP”两个命令到队列
    // 控制器会按顺序执行它们
    AT_controller_cmd_submit(&cmd_set_mode);
    AT_controller_cmd_submit(&cmd_join_ap);
}

void WiFi_disconnect(void) {
    AT_controller_cmd_submit(&cmd_quit_ap);
}

wifi_state_typedef WiFi_get_state(void) {
    return g_wifi_state;
}



#if 0
/**
 * 清空储存的模块wifi信息
 */
void WiFi_init(void){
	ESP_WiFi.ESP_WiFi_Mode 	= 0;
	ESP_WiFi.ESP_WiFi_State = 0;
	memset(&(ESP_WiFi.Wifi_info), 0, sizeof(AP_info_t));
}

/**
 * 设置模块连接的WiFi的信息
 */
void WiFi_info_set(AP_info_t* ap_info){
	ESP_WiFi.Wifi_info.PWD = ap_info->PWD;
	ESP_WiFi.Wifi_info.SSID = ap_info->SSID;
}

/**
 * 查询模块wifi工作模式
 */
AT_CmdResult_t WiFi_mode_get(void){
	static AT_Cmd_t CWMODE = { // 查询模块 Wi-Fi 模式
		.cmd_str = "AT+CWMODE?",
		.data_to_send = NULL,
		.timeout_ms = 100,
		.parser_cb = NULL,
		.response_cb = NULL,
	};
}

/**
 * 设置模块wifi工作模式
 */
AT_CmdResult_t WiFi_mode_set(wifi_mode_typedef mode){
	static char cmd_str[] = "AT+CWMODE=3,0\r\n";
	sprintf(cmd_str, "AT+CWMODE=%d,0\r\n", mode);
	static AT_Cmd_t CWMODE = { // 查询模块 Wi-Fi 模式
		.cmd_str = cmd_str,
		.data_to_send = NULL,
		.timeout_ms = 100,
		.parser_cb = NULL,
		.response_cb = NULL,
	};
}
#endif

