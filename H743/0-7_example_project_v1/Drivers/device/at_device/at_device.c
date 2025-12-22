#include "at_device.h"
#include "SYSTEM/sys.h"
#include <stdio.h>
#include <string.h>

#define AT_MAX_LINE_LEN 256
#define AT_MAX_CMD_LEN 128

typedef enum {
  AT_STATE_IDLE,
  AT_STATE_WAIT_RSP,
} at_state_t;

struct at_device_t {
  at_device_config_t config;
  at_state_t state;
  uint32_t start_time;
  uint32_t current_timeout;
  at_response_cb current_cb;
  void *current_user_data;
  char line_buf[AT_MAX_LINE_LEN];
  uint16_t line_pos;
  uint32_t raw_data_to_read;
};

at_device_t *at_device_create(const at_device_config_t *config) {
  if (!config || !config->uart_queue)
    return NULL;

  at_device_t *device =
      (at_device_t *)sys_malloc(SYS_MEM_INTERNAL, sizeof(at_device_t));
  if (device) {
    memset(device, 0, sizeof(at_device_t));
    device->config = *config;
    device->state = AT_STATE_IDLE;
  }
  return device;
}

void at_device_destroy(at_device_t *device) {
  if (device) {
    sys_free(SYS_MEM_INTERNAL, device);
  }
}

int at_device_send_cmd(at_device_t *device, const char *cmd, uint32_t timeout,
                       at_response_cb cb, void *user_data) {
  if (!device || !cmd || device->state != AT_STATE_IDLE)
    return -1;

  // 发送命令
  size_t len = strlen(cmd);
  if (!uart_queue_send(device->config.uart_queue, (const uint8_t *)cmd, len))
    return -2;
  // 发送换行符 (默认AT协议通常需要)
  uart_queue_send(device->config.uart_queue, (const uint8_t *)"\r\n", 2);

  device->state = AT_STATE_WAIT_RSP;
  device->start_time = sys_get_systick_ms();
  device->current_timeout =
      (timeout > 0) ? timeout : device->config.default_timeout;
  device->current_cb = cb;
  device->current_user_data = user_data;
  device->line_pos = 0;

  return 0;
}

int at_device_send_data(at_device_t *device, const uint8_t *data, size_t len) {
  if (!device || !data || len == 0)
    return -1;
  return uart_queue_send(device->config.uart_queue, data, len) ? 0 : -2;
}

static void handle_at_line(at_device_t *device, const char *line) {
  // 检查 URC (由特定设备驱动解析，如 +IPD)
  if (device->config.urc_handler) {
    device->config.urc_handler(device, line, device->config.user_data);
  }

  if (device->state == AT_STATE_WAIT_RSP) {
    at_result_t res = AT_RESULT_ERROR;
    bool done = false;

    if (strstr(line, "OK")) {
      res = AT_RESULT_OK;
      done = true;
    } else if (strstr(line, "ERROR") || strstr(line, "FAIL")) {
      res = AT_RESULT_ERROR;
      done = true;
    }

    if (done) {
      at_response_cb cb = device->current_cb;
      void *user_data = device->current_user_data;
      device->state = AT_STATE_IDLE;
      device->current_cb = NULL;
      if (cb)
        cb(res, line, user_data);
    }
  }
}

void at_device_process(at_device_t *device) {
  if (!device)
    return;

  // 1. 处理超时
  if (device->state == AT_STATE_WAIT_RSP) {
    if ((sys_get_systick_ms() - device->start_time) > device->current_timeout) {
      at_response_cb cb = device->current_cb;
      void *user_data = device->current_user_data;
      device->state = AT_STATE_IDLE;
      device->current_cb = NULL;
      if (cb)
        cb(AT_RESULT_TIMEOUT, NULL, user_data);
    }
  }

  // 2. 解析行或处理原始数据
  uint8_t ch;
  while (uart_queue_getchar(device->config.uart_queue, &ch)) {
    if (device->raw_data_to_read > 0) {
      if (device->config.raw_handler) {
        device->config.raw_handler(ch, device->config.user_data);
      }
      device->raw_data_to_read--;
      continue;
    }

    if (ch == '\r' || ch == '\n' || ch == '>') {
      if (ch == '>') {
        device->line_buf[device->line_pos++] = ch;
      }
      if (device->line_pos > 0) {
        device->line_buf[device->line_pos] = '\0';
        handle_at_line(device, device->line_buf);
        device->line_pos = 0;
      }
    } else {
      if (device->line_pos < AT_MAX_LINE_LEN - 1) {
        device->line_buf[device->line_pos++] = ch;
      } else {
        // 溢出处理，强行截断并处理一次
        device->line_buf[device->line_pos] = '\0';
        handle_at_line(device, device->line_buf);
        device->line_pos = 0;
      }
    }
  }
}

void at_device_enter_raw_mode(at_device_t *device, uint32_t len) {
  if (device) {
    device->raw_data_to_read = len;
  }
}
