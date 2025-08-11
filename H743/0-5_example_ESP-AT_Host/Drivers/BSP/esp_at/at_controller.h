/*
 * at_controller.h
 *
 *  Created on: Aug 11, 2025
 *      Author: 12114
 */

#ifndef BSP_ESP_AT_AT_CONTROLLER_H_
#define BSP_ESP_AT_AT_CONTROLLER_H_

#include <stdint.h>

// --- 公开函数 ---
void at_controller_init(void);


// --- 由 at_dispatcher.c 调用的处理函数 ---
// 注意：这些函数不是给用户调用的，所以我们把它们放在这里声明，
// 只是为了让 at_dispatcher.c 能看到它们。

// 处理最终响应
void handle_final_ok(const char* line);
void handle_final_error(const char* line);

// 处理URCs
void handle_urc_ipd(const char* line);
void handle_urc_mqtt_recv(const char* line);
void handle_urc_mqtt_connected(const char* line);
void handle_urc_mqtt_disconnected(const char* line);
void handle_urc_ready(const char* line);
void handle_urc_wifi_connected(const char* line);
void handle_urc_wifi_got_ip(const char* line);
void handle_urc_wifi_disconnected(const char* line);

// 处理特殊提示符
void handle_prompt(const char* line);

// 处理数据响应
void handle_data_cwlap(const char* line);
void handle_data_ip_addr(const char* line);

#endif /* BSP_ESP_AT_AT_CONTROLLER_H_ */
