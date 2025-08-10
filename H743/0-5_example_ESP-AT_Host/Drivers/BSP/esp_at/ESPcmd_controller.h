/*
 * ESPcommand_controller.h
 *
 *  Created on: Aug 10, 2025
 *      Author: 12114
 */

#ifndef BSP_ESP_AT_ESPCMD_CONTROLLER_H_
#define BSP_ESP_AT_ESPCMD_CONTROLLER_H_

#include <stdint.h>
#include <stdbool.h>

// 命令执行结果的回调函数指针类型
// context 指针可以用来传递任意用户数据
typedef void (*cmd_callback_t)(void *context, bool success, const char* response_data);

/**
 * @brief 初始化指令控制器
 */
void cmd_controller_init(void);

/**
 * @brief 向队列中添加一个新命令来执行
 *
 * @param cmd 要发送的AT指令字符串 (例如 "AT+CWJAP=\"SSID\",\"PASS\"")
 * @param success_rsp 期望的成功响应字符串 (通常是 "OK")
 * @param timeout_ms 超时时间（毫秒）
 * @param callback 命令完成时的回调函数
 * @param context 传递给回调函数的上下文指针
 * @return true 命令成功入队
 * @return false 命令队列已满
 */
bool cmd_controller_execute(const char* cmd, const char* success_rsp, uint32_t timeout_ms, cmd_callback_t callback, void* context);

/**
 * @brief 在主循环中周期性调用的处理函数
 *        负责驱动状态机、检查超时和发送新命令
 */
void cmd_controller_process(void);

/**
 * @brief 由解析器层调用的函数，用于输入一个完整的消息行
 * @param line 从串口接收并由解析器处理好的一行完整消息
 */
void cmd_controller_handle_line(const char* line);

/**
 * @brief 注册一个URC处理器
 * @param urc_prefix URC的前缀，例如 "+MQTTSUBRECV:"
 * @param handler 处理该URC的回调函数
 */
typedef void (*urc_handler_t)(const char* urc_data);
void cmd_controller_register_urc_handler(const char* urc_prefix, urc_handler_t handler);



#endif /* BSP_ESP_AT_ESPCMD_CONTROLLER_H_ */
