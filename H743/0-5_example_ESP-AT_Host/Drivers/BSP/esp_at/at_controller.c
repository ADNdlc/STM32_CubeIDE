/*
 * at_controller.c
 *
 *  Created on: Aug 11, 2025
 *      Author: 12114
 */

#include "at_uart.h"
#include "at_controller.h"
#include "at_dispatcher.h" // 控制器需要和分发器交互
#ifndef NDEBUG
#include <stdio.h>
#endif

// --- 状态机定义 ---
typedef enum {
    AT_CTRL_STATE_IDLE,			//空闲
    AT_CTRL_STATE_WAIT_RSP,		//命令已发送，等待响应
    AT_CTRL_STATE_WAIT_DATAIN	//输入模式
} AT_CtrlState_t;


// --- 私有变量 ---
static AT_CtrlState_t g_state = AT_CTRL_STATE_IDLE;	//状态机标志
static AT_Cmd_t* g_cmd_queue_head = NULL;			// 队列头
static AT_Cmd_t* g_cmd_queue_tail = NULL;			// 队列尾
static AT_Cmd_t* g_current_cmd = NULL; 				// 当前正在执行的命令
static uint32_t  g_cmd_sent_time = 0;   			// 命令发送的时间戳

static uint8_t timeout_count = 0;
static uint8_t error_count   = 0;



// --- 公开函数实现 ---
void AT_controller_init(void){
	g_state = AT_CTRL_STATE_IDLE;
    // ... 初始化其他状态
}

/**
 * @brief 从命令队列头部取出一个命令
 * @return 指向命令对象的指针，如果队列为空则返回NULL
 */
static AT_Cmd_t* cmd_dequeue(void) {
    if (g_cmd_queue_head == NULL) {
        return NULL;// 队列为空
    }
    AT_Cmd_t* cmd_to_process = g_cmd_queue_head;	// 获取头节点
    g_cmd_queue_head = g_cmd_queue_head->next;		// 将头指针移动到下一个节点
    if (g_cmd_queue_head == NULL) {					// 如果出队后队列变空了，更新尾指针
        g_cmd_queue_tail = NULL;
    }
    cmd_to_process->next = NULL;
    return cmd_to_process;
}

// --- 控制器核心 ---
void AT_controller_process(void) {
    switch (g_state) {
        case AT_CTRL_STATE_IDLE:
            if (g_cmd_queue_head != NULL) {		// 如果空闲且队列中有待处理命令
                g_current_cmd = cmd_dequeue();	// 从队列中取出一个命令

                ATuart_send_string(g_current_cmd->cmd_str);	// 发送AT指令

                g_cmd_sent_time = HAL_GetTick();			// 记录发送时间
                if (g_current_cmd->data_to_send != NULL) { 	// 根据命令类型决定下一个状态
                    g_state = AT_CTRL_STATE_WAIT_DATAIN;
                } else {
                    g_state = AT_CTRL_STATE_WAIT_RSP;
                }
            }
            break;

        case AT_CTRL_STATE_WAIT_RSP:
        case AT_CTRL_STATE_WAIT_DATAIN:
            if (HAL_GetTick()-g_cmd_sent_time > g_current_cmd->timeout_ms) {// 检查超时
                // 超时发生！
                if (g_current_cmd->response_cb) {
                    g_current_cmd->response_cb(AT_CMD_TIMEOUT, "Timeout");//既然是超时自然没有响应,先手动传一个字符串
                }
                g_current_cmd = NULL;
                g_state = AT_CTRL_STATE_IDLE;
                timeout_count ++;
            }
            break;
    }
}

/* ============================ 由分发器调用的处理函数 ============================== */
/**
 * 返回"OK"调用
 */
void handle_final_ok(const char* line) {
    if (g_state == AT_CTRL_STATE_WAIT_RSP) {
        if (g_current_cmd && g_current_cmd->response_cb) {
            g_current_cmd->response_cb(AT_CMD_OK, line);// 调用命令体完成的回调
        }
        g_current_cmd = NULL;
        g_state = AT_CTRL_STATE_IDLE;
    }
#ifndef NDEBUG
	printf("handle_final_ok: success!\r\n");
#endif
}

/**
 * 返回"ERROR"调用
 */
void handle_final_error(const char* line) {
    if (g_state == AT_CTRL_STATE_WAIT_RSP) {
        if (g_current_cmd && g_current_cmd->response_cb) {
            g_current_cmd->response_cb(AT_CMD_ERROR, line);// 调用命令体的完成回调
        }
        g_current_cmd = NULL;
        g_state = AT_CTRL_STATE_IDLE;
        error_count ++;
    }
#ifndef NDEBUG
	printf("handle_final_error: Rcv an ERR\r\n");
#endif
}

/**
 * 等待">"后发送数据(如果有)
 */
void handle_Txdata_send(const char* line) {
    if (g_state == AT_CTRL_STATE_WAIT_DATAIN) {
        // 收到'>'，发送数据
    	if(g_current_cmd->data_to_send == NULL){
    		ATuart_send_string("ERR");	// 让模块退出此状态
    		g_state = AT_CTRL_STATE_WAIT_RSP;
    		error_count ++;
#ifndef NDEBUG
	printf("handle_Txdata_send: NULL to send\r\n");
#endif
    	}else{
    		ATuart_send_string(g_current_cmd->data_to_send);
    		g_state = AT_CTRL_STATE_WAIT_RSP; // 等待最终的 SEND OK/FAIL
    	}
    }
}

/**
 * 接收到“消息报告”类信息的回调,在这里进行数据解析
 */
void handle_Rxdata_process(const char* line) {
    if (g_current_cmd && g_current_cmd->parser_cb) {// 如果当前有命令在执行，并且它定义了数据解析回调
        g_current_cmd->parser_cb(line);
        return;
    }
#ifndef NDEBUG
	printf("handle_Rxdata_process: noone to handle\r\n");
#endif
}

/**
 * 接收到“busy p...”
 * 一般来说，状态机正常运行是不会看到此消息？但我不清楚模块处理云命令时能否响应
 */
void handle_busy(const char* line){
    if (g_current_cmd != NULL) {		// 如果空闲且队列中有待处理命令
         ATuart_send_string(g_current_cmd->cmd_str);	// 发送AT指令
    }
}


void handle_urc_mqtt_recv(const char* line) {
    // 这是关键的云端下发命令处理
    printf("Controller: URC - MQTT message received!\r\n");
    // 在这里解析 line, 提取 topic 和 payload
    // 例如: sscanf(line, "+MQTTSUBRECV:%*d,\"%[^\"]\",%*d,%s", topic, payload);
    // 然后根据内容执行相应操作...
}

// 对于暂时不处理的，可以先留一个空的实现或只打印一条消息
void handle_urc_ipd(const char* line) {  }
void handle_urc_mqtt_connected(const char* line) {  }
void handle_urc_mqtt_disconnected(const char* line) {  }
void handle_urc_ready(const char* line) { printf("Controller: Module is ready.\r\n"); }
void handle_urc_wifi_disconnected(const char* line) {  }
void handle_data_cwlap(const char* line) { printf("Controller: Wi-Fi scan result: %s\r\n", line); }
void handle_data_ip_addr(const char* line) { printf("Controller: IP data: %s\r\n", line); }


