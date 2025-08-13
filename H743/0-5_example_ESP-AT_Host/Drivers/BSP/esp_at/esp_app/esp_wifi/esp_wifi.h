/*
 * esp_wifi.h
 *
 *  Created on: Aug 13, 2025
 *      Author: 12114
 */

#ifndef BSP_ESP_AT_ESP_APP_ESP_WIFI_ESP_WIFI_H_
#define BSP_ESP_AT_ESP_APP_ESP_WIFI_ESP_WIFI_H_

typedef enum {
	WIFI_STATE_UNCONNECTED = 0,
	WIFI_STATE_NO_IP,
	WIFI_STATE_GOT_IP,
	WIFI_STATE_CONNECTING,
	WIFI_STATE_DISCONNECTED,
} wifi_state_typedef;//WiFi连接状态,值一一对应


typedef struct wifi_info_t{//通用WiFi信息,可设置多个可连接WiFi
	char* SSID;		//要连接的WiFi名
	char* PWD;		//要连接的WiFi密码
	//char* BSSID;	//要连接的AP的MAC地址(无重名时可省略)
} wifi_info_t;


// 事件回调函数指针类型
typedef void (*wifi_event_cb_t)(wifi_state_typedef new_state);


// --- 公开API ---

/**
 * @brief 初始化Wi-Fi管理器，并注册一个可选的事件回调函数
 * @param event_callback 当WiFi状态变化时调用的函数，可为NULL
 */
void WiFi_init(wifi_event_cb_t event_callback);

/**
 * @brief 配置模块为Station模式并连接到指定的AP
 *        这是一个非阻塞函数，会立即返回。
 * @param ssid AP的SSID
 * @param pwd AP的密码
 */
void WiFi_connect(wifi_info_t* APdata);

/**
 * @brief 断开当前的Wi-Fi连接
 *        这是一个非阻塞函数。
 */
void WiFi_disconnect(void);

/**
 * @brief 获取当前Wi-Fi的连接状态
 * @return wifi_state_typedef 当前状态
 */
wifi_state_typedef WiFi_get_state(void);


// --- 由Dispatcher调用的内部URC处理函数 ---
// 虽然是公开的，但不应由用户直接调用
void WiFi_handle_urc_connected(const char* line);
void WiFi_handle_urc_got_ip(const char* line);
void WiFi_handle_urc_disconnect(const char* line);

#endif /* BSP_ESP_AT_ESP_APP_ESP_WIFI_ESP_WIFI_H_ */
