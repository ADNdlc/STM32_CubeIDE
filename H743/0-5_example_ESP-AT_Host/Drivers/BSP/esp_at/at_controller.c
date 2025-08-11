/*
 * at_controller.c
 *
 *  Created on: Aug 11, 2025
 *      Author: 12114
 */

#include "at_controller.h"
#include <stdio.h>

// 控制器的状态
typedef enum {
    AT_STATE_IDLE,          // 空闲，可以发送新命令
    AT_STATE_WAIT_RESPONSE  // 已发送命令，等待最终响应 (OK/ERROR)
} AT_ControllerState_t;

// 模块的网络状态（示例）
typedef struct {
    uint8_t wifi_connected;
    uint8_t got_ip;
    uint8_t mqtt_connected;
} ModuleStatus_t;

// --- 私有变量 ---
static volatile AT_ControllerState_t g_controller_state = AT_STATE_IDLE;
static ModuleStatus_t g_module_status = {0};

// --- 公开函数实现 ---
void at_controller_init(void) {
    g_controller_state = AT_STATE_IDLE;
    // ... 初始化其他状态
}

// ========================================================
// 以下是所有处理函数的具体实现
// ========================================================

void handle_final_ok(const char* line) {
    printf("Controller: Received final OK.\r\n");
    // 如果我们正在等待响应，这表示命令成功
    if (g_controller_state == AT_STATE_WAIT_RESPONSE) {
        // TODO: 调用成功回调，处理命令队列
        g_controller_state = AT_STATE_IDLE; // 返回空闲状态
    }
}

void handle_final_error(const char* line) {
    printf("Controller: Received ERROR.\r\n");
    // 如果我们正在等待响应，这表示命令失败
    if (g_controller_state == AT_STATE_WAIT_RESPONSE) {
        // TODO: 调用失败回调，处理命令队列
        g_controller_state = AT_STATE_IDLE; // 返回空闲状态
    }
}

void handle_urc_wifi_connected(const char* line) {
    printf("Controller: URC - WiFi is connected!\r\n");
    g_module_status.wifi_connected = 1;
}

void handle_urc_wifi_got_ip(const char* line) {
    printf("Controller: URC - Got IP address!\r\n");
    g_module_status.got_ip = 1;
}

void handle_urc_mqtt_recv(const char* line) {
    // 这是关键的云端下发命令处理
    printf("Controller: URC - MQTT message received!\r\n");
    // 在这里解析 line, 提取 topic 和 payload
    // 例如: sscanf(line, "+MQTTSUBRECV:%*d,\"%[^\"]\",%*d,%s", topic, payload);
    // 然后根据内容执行相应操作...
}

// 对于暂时不处理的，可以先留一个空的实现或只打印一条消息
void handle_urc_ipd(const char* line) { /* Not handled yet */ }
void handle_urc_mqtt_connected(const char* line) { g_module_status.mqtt_connected = 1; }
void handle_urc_mqtt_disconnected(const char* line) { g_module_status.mqtt_connected = 0; }
void handle_urc_ready(const char* line) { printf("Controller: Module is ready.\r\n"); }
void handle_urc_wifi_disconnected(const char* line) { g_module_status.wifi_connected = 0; g_module_status.got_ip = 0; }
void handle_prompt(const char* line) { printf("Controller: Received prompt >\r\n"); /* TODO: handle data sending state */ }
void handle_data_cwlap(const char* line) { printf("Controller: Wi-Fi scan result: %s\r\n", line); }
void handle_data_ip_addr(const char* line) { printf("Controller: IP data: %s\r\n", line); }
