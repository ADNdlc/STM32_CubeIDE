/*
 * at_dispatcher.c
 *
 *  Created on: Aug 11, 2025
 *      Author: 12114
 */

#include "at_dispatcher.h"
#include "at_controller.h"
#include <string.h>
#include <stdio.h>


/* ============================ 表项结构 ============================== */
typedef void (*at_handler_func_t)(const char* line);// 处理函数原型
typedef struct {									// 处理程序表条目
    const char* prefix;
    at_handler_func_t handler;
} AT_Handler_t;


// ===================================================================
// 核心“处理程序表”
// ===================================================================
// 注意：更具体的前缀应该放在更通用的前缀之前。
// 例如, "+MQTTSUBRECV:" 应该在 "+MQTT" 之前,否则最后strncmp()就会把前者匹配为后者的响应。
static const AT_Handler_t at_handlers[] = {
    // --- 1. 最终响应 (Final Responses) ---
    { "OK",         handle_final_ok },
    { "ERROR",      handle_final_error },
    { "SEND OK",    handle_final_ok }, 		// 对于CIPSEND, SEND OK就是成功标志
    { "SEND FAIL",  handle_final_error },	// 发送失败

    // --- 2. 特殊提示符 ---
	{ "busy p...",	handle_busy },			//模块正在处理上一条命令
    { ">",          handle_CMDdata_send },	//模块进入输入模式

    // --- 3. URCs (非请求结果码) - 可按功能或出现频率分组 ---

//    { "+IPD",               handle_urc_ipd },
//    { "+MQTTSUBRECV:",      handle_urc_mqtt_recv },
//    { "+MQTTCONNECTED",     handle_urc_mqtt_connected },
//    { "+MQTTDISCONNECTED",  handle_urc_mqtt_disconnected },
//    { "ready",              handle_urc_ready },
//    { "WIFI CONNECTED",     handle_urc_wifi_connected },
//    { "WIFI GOT IP",        handle_urc_wifi_got_ip },
//    { "WIFI DISCONNECT",    handle_urc_wifi_disconnected },

    // --- 4. 数据响应 (Data Responses) ---
    // 查询命令返回的具体数据(只有发送命令才会 接受到这些消息)
    { "+CWLAP:",    handle_Rxdata_process }, 	// WiFi扫描结果
    { "+CIPSTA_IP:", handle_Rxdata_process }, 	// 获取到IP地址的另一种方式

	{ "+test_data:", handle_Rxdata_process }

	// ... 在这里可以添加需要处理的其他响应
};

/**
 * 响应分发器函数
 * 接受一个标准字符串(以"\0"结尾),判断类型并调用相应处理函数
 */
void AT_dispatcher_LineProcess(const char* line) {
    // 遍历处理程序表
    for (size_t i = 0; i < sizeof(at_handlers) / sizeof(at_handlers[0]); ++i) {
        // 使用 strncmp 比较前缀
        if (strncmp(line, at_handlers[i].prefix, strlen(at_handlers[i].prefix)) == 0) {
            // 找到匹配项，调用对应的处理函数
            at_handlers[i].handler(line);
            return; // 处理完毕，退出函数
        }
    }
    // 如果循环结束都没有找到匹配项，说明这是一个不关心的消息。
#ifndef NDEBUG
    printf("Unhandled line: [%s]\r\n", line);
#endif
}

/**
 * 行解析回调函数
 * 解析到完整的行后调用"响应分发器"
 */
void AT_parser_line(const char* line) {
    // 解析器的消息
#ifndef NDEBUG
	printf("parser_line:%s \r\n",line);
#endif
	AT_dispatcher_LineProcess(line);
}

