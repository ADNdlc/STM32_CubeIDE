/*
 * at_controller.h
 *
 *  Created on: Aug 11, 2025
 *      Author: 12114
 */

#ifndef DEVICE_ESP_8266_AT_CONTROLLER_H_
#define DEVICE_ESP_8266_AT_CONTROLLER_H_

#include <stdbool.h>
#include <stdint.h>

/* ===================== Forward Declarations ===================== */
typedef struct uart_queue_t uart_queue_t;
typedef struct gpio_driver_t gpio_driver_t;

/* ===================== Type Definitions ===================== */

// --- 命令执行结果 ---
typedef enum {
  AT_CMD_OK = 0,  // Successfully received "OK" or "SEND OK"
  AT_CMD_ERROR,   // Received "ERROR" or "SEND FAIL"
  AT_CMD_TIMEOUT, // 超时
  AT_CMD_ABORT,   // 终止 (e.g., during reset)
} AT_CmdResult_t;

// --- 状态机状态 ---
typedef enum {
  AT_CTRL_STATE_RESETTING = 0, // 复位中
  AT_CTRL_STATE_WAIT_READY,    // 等待模块就绪
  AT_CTRL_STATE_IDLE,          // 空闲，准备发送命令
  AT_CTRL_STATE_WAIT_RSP,      // 等待模块响应
  AT_CTRL_STATE_WAIT_DATAIN,   // 等待模块进入数据输入模式(">")
  AT_CTRL_STATE_BUSY,          // 模块忙碌("busy p...")
} AT_CtrlState_t;

// --- 命令体回调原型 ---
// 数据查询命令回调 (e.g., +IPD, +CWMODE:...)
typedef void (*at_parser_cb_t)(void *ctx, const char *line);
// 命令执行结果回调: handle command result (OK/ERROR/TIMEOUT)
typedef void (*at_response_cb_t)(void *ctx, AT_CmdResult_t result,
                                 const char *last_line);

// --- 命令对象 ---
typedef struct AT_Cmd_t {
  const char *cmd_str;          // 命令内容 (e.g., "AT+GMR\r\n")
  const char *data_to_send;     // 需要发送的数据,可选 (如密码或网络通信内容)
  uint32_t timeout_ms;          // 命令执行超时时间
  at_parser_cb_t parser_cb;     // 查询命令回调
  at_response_cb_t response_cb; // 执行结果回调
  void *ctx;                    // parser_cb上下文

  struct AT_Cmd_t *next; // 链表
} AT_Cmd_t;

// --- URC (Unsolicited Result Code) 处理表 ---
// 处理函数原型
typedef void (*at_handler_func_t)(void *ctx, const char *line);
// URC 处理节点
typedef struct at_urc_node_t {
  const char *prefix;
  at_handler_func_t handler;
  void *ctx; // 上下文
  struct at_urc_node_t *next;
} at_urc_node_t;

// --- AT控制器对象 ---
typedef struct at_controller_t {
  // 依赖的对象
  uart_queue_t *uart;     // UART队列
  gpio_driver_t *rst_pin; // GPIO用于硬复位

  // State
  AT_CtrlState_t state;      // 状态机状态
  uint32_t last_action_time; // 开始等待时间
  uint32_t wait_sent_time;   // 模块发送数据等待时间

  // 命令队列
  AT_Cmd_t *cmd_queue_head; // 命令队列头
  AT_Cmd_t *cmd_queue_tail; // 命令队列尾
  AT_Cmd_t *current_cmd;    // 正在执行的命令

  // URC 处理表
  at_urc_node_t *urc_list; // URC处理链表头

  // 错误处理
  uint32_t error_count;          // Error 计数
  uint32_t consecutive_timeouts; // 连续超时计数(无响应)
  uint32_t busy_count;           // "busy p..."计数
} at_controller_t;

/* ===================== API ===================== */

/**
 * @brief 初始化 AT控制器
 * @param self 对象实例
 * @param uart 初始化好的UART队列
 * @param rst_pin 模块复位引脚
 */
void at_controller_init(at_controller_t *self, uart_queue_t *uart,
                        gpio_driver_t *rst_pin);

/**
 * @brief 控制器运行处理
 * @param self 实例
 */
void at_controller_process(at_controller_t *self);

/**
 * @brief 向控制器提交AT命令
 *        命令会被拷贝并分配空间
 * @param self 控制器实例
 * @param cmd 命令体
 * @return 0 on success, non-zero on error
 */
int at_controller_submit_cmd(at_controller_t *self, AT_Cmd_t *cmd);

/**
 * @brief 应用层错误报告
 *        This may trigger a system reset/recovery if the fault threshold is
 * reached.
 * @param self 控制器实例
 */
void at_controller_report_fault(at_controller_t *self);

/* ===================== at_dispatcher ===================== */
/**
 * @brief 注册一个 URC处理函数
 * @param self 控制器实例
 * @param prefix 消息标识前缀 (e.g., "+MQTTCONNECTED")
 * @param handler 回调
 * @return 0 on success
 */
int at_controller_register_handler(at_controller_t *self, const char *prefix,
                                   at_handler_func_t handler, void *ctx);

/**
 * @brief Dispatch a received line to registered handlers
 *        (Internal use, but exposed for the parser)
 * @param self Controller instance
 * @param line Received line string
 * @return true if handled by a URC handler, false otherwise
 */
bool at_controller_dispatch(at_controller_t *self, const char *line);

#endif /* DEVICE_ESP_8266_AT_CONTROLLER_H_ */
