/*
 * at_controller.c
 *
 *  Created on: Aug 11, 2025
 *      Author: 12114
 */

#define LOG_TAG "at_ctrl"
#include "at_controller.h"
#include <stdio.h>
#include <string.h>

// 依赖 (使用完整相对路径确保编译环境无关性)
#include "../../../Component/uart_queue/uart_queue.h"
#include "../../../EasyLogger/easylogger/inc/elog.h"
#include "../../SYSTEM/sys.h"
#include "../../interface/gpio_driver.h"

// --- Internal Prototypes ---
static void at_controller_reset_hw(at_controller_t *self);
static void at_controller_process_cmd(at_controller_t *self);
static void at_cmd_cleanup(at_controller_t *self, AT_Cmd_t *cmd);

/* ===================== 内部处理回调 ===================== */
// "OK"
static void handle_final_ok(void *ctx, const char *line) {
  at_controller_t *self = (at_controller_t *)ctx;
  if (self->state == AT_CTRL_STATE_WAIT_RSP ||
      self->state == AT_CTRL_STATE_WAIT_DATAIN) {
    if (self->current_cmd && self->current_cmd->response_cb) {
      self->current_cmd->response_cb(AT_CMD_OK, line);
    }
    log_d("Cmd OK: %s",
          self->current_cmd ? self->current_cmd->cmd_str : "NULL");
    at_cmd_cleanup(self, self->current_cmd);
    self->current_cmd = NULL;
    self->state = AT_CTRL_STATE_IDLE;
    self->busy_count = 0;
    self->consecutive_timeouts = 0;
  }
}
// "ERROR"
static void handle_final_error(void *ctx, const char *line) {
  at_controller_t *self = (at_controller_t *)ctx;
  if (self->state == AT_CTRL_STATE_WAIT_RSP ||
      self->state == AT_CTRL_STATE_WAIT_DATAIN) {
    log_e("Cmd Error: %s",
          self->current_cmd ? self->current_cmd->cmd_str : "NULL");
    if (self->current_cmd && self->current_cmd->response_cb) {
      self->current_cmd->response_cb(AT_CMD_ERROR, line);
    }

    at_cmd_cleanup(self, self->current_cmd);
    self->current_cmd = NULL;
    self->state = AT_CTRL_STATE_IDLE;
  }
}
// "busy p..."
static void handle_busy(void *ctx, const char *line) {
  at_controller_t *self = (at_controller_t *)ctx;
  if (self->current_cmd != NULL) {
    log_w("Module busy, retry %d", self->busy_count + 1);
    self->busy_count++;
    self->state = AT_CTRL_STATE_BUSY;
    self->wait_sent_time = sys_get_systick_ms();
  }
}
// "ready"
static void handle_ready(void *ctx, const char *line) {
  at_controller_t *self = (at_controller_t *)ctx;
  log_i("Module ready");
  if (self->state == AT_CTRL_STATE_WAIT_READY ||
      self->state == AT_CTRL_STATE_RESETTING) {
    self->state = AT_CTRL_STATE_IDLE;
    self->error_count = 0;
    self->consecutive_timeouts = 0;
  }
}
// ">"
static void handle_CMDdata_send(void *ctx, const char *line) {
  at_controller_t *self = (at_controller_t *)ctx;
  if (self->state == AT_CTRL_STATE_WAIT_DATAIN && self->current_cmd &&
      self->current_cmd->data_to_send) {
    uart_queue_send(self->uart,
                    (const uint8_t *)self->current_cmd->data_to_send,
                    strlen(self->current_cmd->data_to_send));
    self->state = AT_CTRL_STATE_WAIT_RSP;
  }
}

// static void handle_Rxdata_process(void *ctx, const char *line) {
//   at_controller_t *self = (at_controller_t *)ctx;
//   if (self->current_cmd && self->current_cmd->parser_cb) {
//     self->current_cmd->parser_cb(line);
//   }
// }

static void at_cmd_cleanup(at_controller_t *self, AT_Cmd_t *cmd) {
  if (!cmd)
    return;
  if (cmd->cmd_str)
    sys_free(SYS_MEM_INTERNAL, (void *)cmd->cmd_str);
  if (cmd->data_to_send)
    sys_free(SYS_MEM_INTERNAL, (void *)cmd->data_to_send);
  sys_free(SYS_MEM_INTERNAL, cmd);
}

/* ===================== API Implementation ===================== */

void at_controller_init(at_controller_t *self, uart_queue_t *uart,
                        gpio_driver_t *rst_pin) {
  if (!self || !uart)
    return;

  memset(self, 0, sizeof(at_controller_t));
  self->uart = uart;
  self->rst_pin = rst_pin;
  self->state = AT_CTRL_STATE_RESETTING;
  self->last_action_time = sys_get_systick_ms();

  log_i("AT Controller Initializing...");

  // 注册基本urc处理
  at_controller_register_handler(self, "ready", handle_ready, self);
  at_controller_register_handler(self, "busy p...", handle_busy, self);
  at_controller_register_handler(self, ">", handle_CMDdata_send, self);
  at_controller_register_handler(self, "OK", handle_final_ok, self);
  at_controller_register_handler(self, "SEND OK", handle_final_ok, self);
  at_controller_register_handler(self, "ERROR", handle_final_error, self);
  at_controller_register_handler(self, "SEND FAIL", handle_final_error, self);

  // at_controller_register_handler(self, "+CWMODE:", handle_Rxdata_process);
  // //wifi at_controller_register_handler(self, "+CIPSNTPTIME:",
  // handle_Rxdata_process); //SNTP

  at_controller_reset_hw(self); // 模块硬重置
}

/**
 * 模块硬件复位
 */
static void at_controller_reset_hw(at_controller_t *self) {
  log_i("Resetting ESP8266 hardware...");
  if (self->rst_pin) {
    GPIO_WRITE(self->rst_pin, 0); // 低电平复位
    sys_delay_ms(100);
    GPIO_WRITE(self->rst_pin, 1);
  } else { // 若无硬件复位，则软件复位(可能失败或一直收不到read)
    log_w("No RST pin, using AT+RST");
    uart_queue_send(self->uart, (uint8_t *)"AT+RST\r\n", 8);
  }
  self->state = AT_CTRL_STATE_WAIT_READY;
  self->wait_sent_time = sys_get_systick_ms();
}

int at_controller_submit_cmd(at_controller_t *self, AT_Cmd_t *cmd) {
  if (!self || !cmd)
    return -1;

  AT_Cmd_t *new_cmd =
      (AT_Cmd_t *)sys_malloc(SYS_MEM_INTERNAL, sizeof(AT_Cmd_t));
  if (!new_cmd)
    return -1;
  *new_cmd = *cmd;
  new_cmd->next = NULL;

  if (cmd->cmd_str) {
    size_t len = strlen(cmd->cmd_str) + 1;
    char *str_copy = (char *)sys_malloc(SYS_MEM_INTERNAL, len);
    if (str_copy) {
      memcpy(str_copy, cmd->cmd_str, len);
      new_cmd->cmd_str = str_copy;
    }
  }

  if (cmd->data_to_send) {
    size_t len = strlen(cmd->data_to_send) + 1;
    char *data_copy = (char *)sys_malloc(SYS_MEM_INTERNAL, len);
    if (data_copy) {
      memcpy(data_copy, cmd->data_to_send, len);
      new_cmd->data_to_send = data_copy;
    }
  }

  if (self->cmd_queue_head == NULL) {
    self->cmd_queue_head = new_cmd;
    self->cmd_queue_tail = new_cmd;
  } else {
    self->cmd_queue_tail->next = new_cmd;
    self->cmd_queue_tail = new_cmd;
  }

  log_v("Cmd submitted: %s", cmd->cmd_str);
  return 0;
}

void at_controller_process(at_controller_t *self) {
  if (!self)
    return;

  static char rx_line_buf[512];
  static uint16_t rx_idx = 0;

  // 接收处理
  uint8_t ch;
  while (uart_queue_getdata(self->uart, &ch, 1) > 0) {
    if (rx_idx >= sizeof(rx_line_buf) - 1) {
      rx_idx = 0;
    }

    rx_line_buf[rx_idx++] = ch;

    if (ch == '\n') {
      rx_line_buf[rx_idx] = '\0';
      if (rx_idx > 1 && rx_line_buf[rx_idx - 2] == '\r') {
        rx_line_buf[rx_idx - 2] = '\0';
      } else if (rx_idx > 0 && rx_line_buf[rx_idx - 1] == '\n') {
        rx_line_buf[rx_idx - 1] = '\0';
        if (rx_idx > 1 && rx_line_buf[rx_idx - 2] == '\r') {
          rx_line_buf[rx_idx - 2] = '\0';
        }
      }
      // 收到一个完整的行(换行符分割)
      if (strlen(rx_line_buf) > 0) {
        log_d("Rx line: %s", rx_line_buf);

        // 1. Check for command echo (ignore if it matches the current command)
        bool is_echo = false;
        if (self->current_cmd && self->current_cmd->cmd_str) {
          // Echo might have extra whitespace or be exactly the command
          if (strncmp(rx_line_buf, self->current_cmd->cmd_str,
                      strlen(self->current_cmd->cmd_str) - 2) == 0) {
            is_echo = true;
            log_v("Echo ignored: %s", rx_line_buf);
          }
        }

        if (!is_echo) {
          // 2. Dispatch to URC handlers
          if (!at_controller_dispatch(self, rx_line_buf)) {
            // 3. If not a URC, and we are waiting for response, pass to command
            // parser
            if (self->current_cmd && self->current_cmd->parser_cb &&
                (self->state == AT_CTRL_STATE_WAIT_RSP ||
                 self->state == AT_CTRL_STATE_WAIT_DATAIN)) {
              self->current_cmd->parser_cb(self->current_cmd->ctx, rx_line_buf);
            } else {
              log_w("Unhandled line: %s", rx_line_buf);
            }
          }
        }
      }
      rx_idx = 0;
    }
    // 收到'>'(无换行符)
    else if (ch == '>') {
      log_d("Rx prompt: >");
      char prompt[2] = {'>', '\0'};
      at_controller_dispatch(self, prompt);
      rx_idx = 0;
    }
  }

  uint32_t now = sys_get_systick_ms();

  // 命令处理
  switch (self->state) {
  case AT_CTRL_STATE_IDLE:
    at_controller_process_cmd(self);
    break;

  case AT_CTRL_STATE_WAIT_RSP:
  case AT_CTRL_STATE_WAIT_DATAIN:
    if (self->current_cmd &&
        (now - self->wait_sent_time > self->current_cmd->timeout_ms)) {
      log_e("Cmd Timeout (%d ms): %s", self->current_cmd->timeout_ms,
            self->current_cmd->cmd_str);
      if (self->current_cmd->response_cb) {
        self->current_cmd->response_cb(AT_CMD_TIMEOUT, "TIMEOUT"); // 超时
      }

      at_cmd_cleanup(self, self->current_cmd); // 释放当前命令
      self->current_cmd = NULL;
      self->state = AT_CTRL_STATE_IDLE;

      self->error_count++;
      self->consecutive_timeouts++;

      if (self->consecutive_timeouts > 5) {
        at_controller_report_fault(self);
      }
    }
    break;

  case AT_CTRL_STATE_BUSY:
    if (now - self->wait_sent_time > 200) {
      if (self->current_cmd) {
        uart_queue_send(self->uart, (uint8_t *)self->current_cmd->cmd_str,
                        strlen(self->current_cmd->cmd_str));
        self->wait_sent_time = now;

        if (self->busy_count * 200 > 10000) {
          if (self->current_cmd->response_cb) {
            self->current_cmd->response_cb(AT_CMD_TIMEOUT, "BUSY TIMEOUT");
          }
          at_cmd_cleanup(self, self->current_cmd);
          self->state = AT_CTRL_STATE_IDLE;
          self->current_cmd = NULL;
        }
      } else {
        self->state = AT_CTRL_STATE_IDLE;
      }
    }
    break;

  case AT_CTRL_STATE_WAIT_READY:
    if (now - self->wait_sent_time > 5000) {
      at_controller_reset_hw(self); // 模块硬重置
    }
    break;

  case AT_CTRL_STATE_RESETTING:
    break;
  }
}

// 处理命令队列
static void at_controller_process_cmd(at_controller_t *self) {
  if (self->cmd_queue_head) { // 有命令
    self->current_cmd = self->cmd_queue_head;
    self->cmd_queue_head = self->cmd_queue_head->next;
    if (self->cmd_queue_head == NULL) { // 队列为空
      self->cmd_queue_tail = NULL;
    }
    // 发送命令(推送到uart队列)
    log_i("Send Cmd -> %s", self->current_cmd->cmd_str);
    uart_queue_send(self->uart, (uint8_t *)self->current_cmd->cmd_str,
                    strlen(self->current_cmd->cmd_str));

    self->wait_sent_time = sys_get_systick_ms();
    if (self->current_cmd->data_to_send != NULL) {
      self->state = AT_CTRL_STATE_WAIT_DATAIN;
    } else {
      self->state = AT_CTRL_STATE_WAIT_RSP;
    }
  }
}

void at_controller_report_fault(at_controller_t *self) {
  self->error_count++;
  log_w("Fault reported: %d/10", self->error_count);
  if (self->error_count > 10) {
    log_e("Fault threshold reached, force reset!");
    at_controller_reset_hw(self);
    self->error_count = 0;
  }
}
