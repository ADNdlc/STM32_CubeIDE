/*
 * at_controller.h
 *
 *  Created on: Aug 11, 2025
 *      Author: 12114
 */

#ifndef BSP_ESP_AT_AT_CONTROLLER_H_
#define BSP_ESP_AT_AT_CONTROLLER_H_

#include <stdint.h>

#define SAVE_CMD		1	//队列是否存储传入命令？

// 命令执行结果
typedef enum {
    AT_CMD_OK,		//成功
    AT_CMD_ERROR,	//错误
    AT_CMD_TIMEOUT	//超时
} AT_CmdResult_t;


// 定义命令对象结构体
typedef struct AT_Cmd_t {
    const char* cmd_str;        // 要发送的AT指令字符串,如"AT+CIPSNTP?"
    const char* data_to_send;   // 对于需要'>'提示符的命令，这是要发送的数据
    uint32_t timeout_ms;        // 此命令的超时时间

    // --- 通用回调函数 ---

    //    命令执行完毕的回调 (成功/失败/超时)
    //    第一个参数是结果，第二个参数是原始的响应行
    void (*response_cb)(AT_CmdResult_t result, const char* line);

    //    数据解析回调
    //    当收到一个非最终响应的数据行时调用
    //    例如，对于"AT+CIPSNTPTIME?"，它会被 "+CIPSNTPTIME:..." 这行触发
    void (*parser_cb)(const char* data_line);

    // 用于构成链表/队列的指针
    struct AT_Cmd_t* next;
} AT_Cmd_t;


/* ============================ 由分发器调用的处理函数 ============================== */
void handle_final_ok(const char* line);
void handle_final_error(const char* line);
void handle_CMDdata_send(const char* line);
void handle_Rxdata_process(const char* line);
void handle_busy(const char* line);
void handle_ready(const char* line);

/* ============================ 应用层函数 ============================== */
/**
 * @brief 初始化控制器
 */
void AT_controller_init(void);

/**
 * @brief 		向控制器提交一个要执行的命令
 * @param cmd 	指向一个命令对象的指针。注意：若没启用 SAVE_CMD 此对象必须是静态的或全局的，
 *            	因为它是异步执行的，不能是栈上的局部变量。
 * @return 0 成功加入队列, -1 队列已满
 */
int AT_controller_cmd_submit(AT_Cmd_t* cmd);

/**
 * @brief 在主循环中周期性调用的控制器处理函数
 *        负责驱动状态机、发送命令、检查超时
 */
void AT_controller_process(void);





// --- 由 at_dispatcher.c 调用的处理函数 ---
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
