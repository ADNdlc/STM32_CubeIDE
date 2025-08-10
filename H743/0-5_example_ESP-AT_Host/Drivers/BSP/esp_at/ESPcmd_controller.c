/*
 * ESPcmd_controller.c
 *
 *  Created on: Aug 10, 2025
 *      Author: 12114
 */


#include "ESPcmd_controller.h"
#include <string.h>
#include <stdio.h> // for debug

// --- 内部数据结构和状态定义 ---

// 控制器状态
typedef enum {
    CMD_STATE_IDLE,         // 空闲，可以发送新命令
    CMD_STATE_WAIT_RSP,     // 已发送命令，等待最终响应
    CMD_STATE_WAIT_PROMPT   // 等待'>'提示符
} cmd_state_t;

// 命令队列中的单个命令结构体
typedef struct {
    char cmd_str[128];
    char success_rsp[16];
    uint32_t timeout_ms;
    cmd_callback_t callback;
    void* context;
    uint32_t start_time;
} cmd_t;

// URC处理器结构体
#define MAX_URC_HANDLERS 10
typedef struct {
    const char* prefix;
    urc_handler_t handler;
} urc_mapping_t;


// --- 模块私有变量 ---

#define CMD_QUEUE_SIZE 8
static cmd_t cmd_queue[CMD_QUEUE_SIZE];
static uint8_t queue_head = 0;
static uint8_t queue_tail = 0;
static uint8_t queue_count = 0;

static cmd_state_t current_state = CMD_STATE_IDLE;
static cmd_t current_cmd; // 当前正在执行的命令

static urc_mapping_t urc_handlers[MAX_URC_HANDLERS];
static uint8_t num_urc_handlers = 0;


// --- 核心实现 ---

void cmd_controller_init(void) {
    // 初始化所有变量
    memset(cmd_queue, 0, sizeof(cmd_queue));
    queue_head = 0;
    queue_tail = 0;
    queue_count = 0;
    current_state = CMD_STATE_IDLE;
    num_urc_handlers = 0;
}

bool cmd_controller_execute(const char* cmd, const char* success_rsp, uint32_t timeout_ms, cmd_callback_t callback, void* context) {
    if (queue_count >= CMD_QUEUE_SIZE) {
        return false; // 队列满
    }

    // 将命令信息存入队列尾部
    strncpy(cmd_queue[queue_tail].cmd_str, cmd, sizeof(cmd_queue[queue_tail].cmd_str) - 1);
    strncpy(cmd_queue[queue_tail].success_rsp, success_rsp, sizeof(cmd_queue[queue_tail].success_rsp) - 1);
    cmd_queue[queue_tail].timeout_ms = timeout_ms;
    cmd_queue[queue_tail].callback = callback;
    cmd_queue[queue_tail].context = context;

    queue_tail = (queue_tail + 1) % CMD_QUEUE_SIZE;
    queue_count++;

    return true;
}

// 在main.c中，需要将at_parser的回调函数设置为本函数
// at_parser.c: extern void cmd_controller_handle_line(const char* line);
//              process_at_line -> cmd_controller_handle_line
void cmd_controller_handle_line(const char* line) {
    // --- URC 分发器 ---
    // 优先检查是否为URC，因为URC可以随时出现
    for (int i = 0; i < num_urc_handlers; i++) {
        if (strncmp(line, urc_handlers[i].prefix, strlen(urc_handlers[i].prefix)) == 0) {
            // 匹配到URC，调用其处理器
            urc_handlers[i].handler(line);
            return; // URC处理完毕，直接返回，不影响当前命令状态
        }
    }

    // --- 命令响应处理器 ---
    if (current_state == CMD_STATE_WAIT_RSP) {
        // 只有在等待响应的状态下，才关心OK/ERROR等
        if (strcmp(line, current_cmd.success_rsp) == 0) {
            // 成功
            if (current_cmd.callback) {
                current_cmd.callback(current_cmd.context, true, NULL); // 成功，无特定数据返回
            }
            current_state = CMD_STATE_IDLE; // 返回空闲状态
        } else if (strcmp(line, "ERROR") == 0 || strcmp(line, "FAIL") == 0) {
            // 失败
            if (current_cmd.callback) {
                current_cmd.callback(current_cmd.context, false, NULL);
            }
            current_state = CMD_STATE_IDLE; // 返回空闲状态
        } else {
            // 其他数据响应，例如 "+CWJAP:..."
            // 这里可以扩展，将此数据行通过回调传递出去
            // printf("Data Response: %s\n", line);
        }
    }
    // 注意：如果当前状态是IDLE，但收到了非URC消息（例如 stray "OK"），我们直接忽略它。
}


void cmd_controller_process(void) {
    // 1. 状态机驱动
    if (current_state == CMD_STATE_IDLE && queue_count > 0) {
        // 从队列头取出一个命令
        current_cmd = cmd_queue[queue_head];
        queue_head = (queue_head + 1) % CMD_QUEUE_SIZE;
        queue_count--;

        // 发送命令
        printf("CMD -> %s\r\n", current_cmd.cmd_str); // 替换为你的uart_send_string函数

        // 更新状态和超时计时器
        current_cmd.start_time = HAL_GetTick(); // 获取当前系统时间
        current_state = CMD_STATE_WAIT_RSP;
    }

    // 2. 超时检查
    if (current_state == CMD_STATE_WAIT_RSP) {
        if (HAL_GetTick() - current_cmd.start_time > current_cmd.timeout_ms) {
            // 超时发生
            printf("CMD TIMEOUT: %s\n", current_cmd.cmd_str);
            if (current_cmd.callback) {
                current_cmd.callback(current_cmd.context, false, "TIMEOUT");
            }
            current_state = CMD_STATE_IDLE; // 返回空闲状态，准备处理下一条命令
        }
    }
}

void cmd_controller_register_urc_handler(const char* urc_prefix, urc_handler_t handler) {
    if (num_urc_handlers < MAX_URC_HANDLERS) {
        urc_handlers[num_urc_handlers].prefix = urc_prefix;
        urc_handlers[num_urc_handlers].handler = handler;
        num_urc_handlers++;
    }
}
