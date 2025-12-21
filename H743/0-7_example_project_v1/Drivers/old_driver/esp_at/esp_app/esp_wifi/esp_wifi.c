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
#if USE_MY_MALLOC
#include "malloc/malloc.h"
#endif

// --- 私有状态和回调 ---
static wifi_mode_typedef  g_mode_state = Closed;				//储存模块工作模式
static wifi_state_typedef g_wifi_state = WIFI_STATE_UNCONNECTED;//模块WiFi连接状态
static wifi_event_cb_t 	  g_state_event_cb = NULL;				//状态改变时调用,由上层实现,可用来通知上层

// --- 私有命令对象和缓冲区 ---
#if !SAVE_CMD
static AT_Cmd_t cmd_set_mode;	// 用于AT+CWMODE=1
static AT_Cmd_t cmd_iqe_mode;	//用于模式查询
static AT_Cmd_t cmd_join_ap;	// 用于AT+CWJAP="ssid","pwd"
static AT_Cmd_t cmd_quit_ap;	// 用于AT+CWQAP
// 用于动态构建命令的缓冲区
static char wifi_info_cmd_buffer[64];
static char wifi_mode_cmd_buffer[20];
#endif

/* 更新内部记录的模块wifi连接状态
 */
static void WiFi_update_state(wifi_state_typedef new_state) {
    if (g_wifi_state != new_state) {
        g_wifi_state = new_state;
#ifndef NDEBUG
        printf("WiFi update_state to: %d\r\n", new_state);
#endif
        // 如果上层注册了回调，就调用它
        if (g_state_event_cb) {
        	g_state_event_cb(new_state);
        }
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

// 模式设置结果回调
static void _setmode_rsp_cb(AT_CmdResult_t result, const char* line) {
    if (result == AT_CMD_OK) {
    } else {
        // 如果命令本身就失败了 (例如参数错误), 则连接失败
    	g_mode_state = Closed;
    }
}

//模式查询回调+CWMODE:
void _inquire_pas_cb(const char* line) {
#ifndef NDEBUG
	printf("mode inquire:%s\r\n",line);
#endif
	g_mode_state = *(line+8) - 48;//Ascll转整形
	if(g_mode_state > 3 || g_mode_state < 0){
#ifndef NDEBUG
	printf("ERR g_mode_state:%d\r\n",g_mode_state);
#endif
		g_mode_state = Closed;
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
#if !SAVE_CMD
    // 初始化所有命令对象
    cmd_set_mode = (AT_Cmd_t){
        .cmd_str = wifi_mode_cmd_buffer,//station,禁止自动连接AP
        .timeout_ms = 500,
        .response_cb = _setmode_rsp_cb,
    };
    cmd_iqe_mode = (AT_Cmd_t){
        .cmd_str = "AT+CWMODE?\r\n",//查询状态
        .timeout_ms = 500,
		.parser_cb = _inquire_pas_cb,
        .response_cb = _setmode_rsp_cb,
    };
    cmd_join_ap = (AT_Cmd_t){
        .cmd_str = wifi_info_cmd_buffer, // 指向动态缓冲区
        .timeout_ms = 10000, // 连接WiFi超时时间需要长一些
        .response_cb = _connect_rsp_cb,
    };
    cmd_quit_ap = (AT_Cmd_t){
        .cmd_str = "AT+CWQAP\r\n",
        .timeout_ms = 1000,
        .response_cb = NULL,
    };
#endif
#ifndef NDEBUG
    printf("WiFi_init succes!\r\n");
#endif
}

/**
 * 发送连接命令
 * 此函数不处理执行结果,它只负责提交命令
 */
void WiFi_connect(wifi_info_t* APdata) {
    // 动态构建AT+CWJAP命令
#if SAVE_CMD
#if USE_MY_MALLOC
	char* wifi_info_cmd_buffer = mymalloc(SRAMDTCM,64);
#else
	char wifi_info_cmd_buffer[64];
#endif
	AT_Cmd_t cmd_join_ap = (AT_Cmd_t){
        .cmd_str = wifi_info_cmd_buffer, // 指向动态缓冲区
        .timeout_ms = 10000, // 连接WiFi超时时间需要长一些
        .response_cb = _connect_rsp_cb,
    };
#endif
	if((g_mode_state==Station)||(g_mode_state==Mixed)){
		snprintf(wifi_info_cmd_buffer, 64, "AT+CWJAP=\"%s\",\"%s\"\r\n", APdata->SSID, APdata->PWD);
		WiFi_update_state(WIFI_STATE_CONNECTING);// 更新状态为“正在连接”
    	AT_controller_cmd_submit(&cmd_join_ap);
    	return;
	}
#ifndef NDEBUG
	printf("WiFi MODE ERR!!!\r\n");
#endif
}

/**
 * 设置模块的工作模式,并查询模块模式判断是否执行成功
 */
void WiFi_set_mode(wifi_mode_typedef wifi_mode) {				//第二个命令参数是自动重连,禁用
#if SAVE_CMD
#if USE_MY_MALLOC
	char* wifi_mode_cmd_buffer = mymalloc(SRAMDTCM,20);
#else
	char wifi_mode_cmd_buffer[20];
#endif
	AT_Cmd_t cmd_set_mode = (AT_Cmd_t){
        .cmd_str = wifi_mode_cmd_buffer, // 指向动态缓冲区
        .timeout_ms = 10000, // 连接WiFi超时时间需要长一些
        .response_cb = _connect_rsp_cb,
    };
	AT_Cmd_t cmd_iqe_mode = (AT_Cmd_t){
	.cmd_str = "AT+CWMODE?\r\n",//查询状态
	.timeout_ms = 500,
	.parser_cb = _inquire_pas_cb,
	.response_cb = _setmode_rsp_cb,
	};
#endif
    snprintf(wifi_mode_cmd_buffer, 20, "AT+CWMODE=%d,0\r\n", wifi_mode);
    AT_controller_cmd_submit(&cmd_set_mode);//设置
    AT_controller_cmd_submit(&cmd_iqe_mode);//查询
}

void WiFi_disconnect(void) {
#if SAVE_CMD
	AT_Cmd_t cmd_quit_ap = (AT_Cmd_t){
	.cmd_str = "AT+CWQAP\r\n",
	.timeout_ms = 1000,
	.response_cb = NULL,
	};
#endif
    AT_controller_cmd_submit(&cmd_quit_ap);
}

wifi_state_typedef WiFi_get_state(void) {
    return g_wifi_state;
}

wifi_mode_typedef WiFi_get_mode(void){
    return g_mode_state;
}
