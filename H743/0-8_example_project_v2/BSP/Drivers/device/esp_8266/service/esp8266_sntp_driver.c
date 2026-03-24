/*
 * esp8266_sntp_driver.c
 *
 *  Implementation of the ESP8266 SNTP driver.
 */

#include "esp8266_sntp_driver.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "esp8266_sntp"
#include "elog.h"

// --- Forward Declarations ---
static int esp8266_sntp_config(sntp_driver_t *base, int timezone,
                               const char *server1);
static int esp8266_sntp_start_sync(sntp_driver_t *base, sntp_sync_cb_t cb,
                                   void *user_data);
static sntp_drv_status_t esp8266_sntp_get_status(sntp_driver_t *base);

static const sntp_driver_ops_t esp8266_sntp_ops = {
    .config = esp8266_sntp_config,
    .start_sync = esp8266_sntp_start_sync,
    .get_status = esp8266_sntp_get_status,
};

// --- URC 处理 ---
static void handle_time_updated(void *ctx, const char *line) {
  esp8266_sntp_driver_t *self = (esp8266_sntp_driver_t *)ctx;
  self->status = SNTP_DRV_STATUS_SUCCESS;
  log_i("SNTP Status: Synchronized (URC)");
}

// 辅助函数: 解析时间字符串
static void parse_sntp_to_rtc(const char *str, sntp_time_t *out) {
  char week[4], month[4];
  int day, hour, min, sec, year;
  // Format: "Fri May 30 18:06:18 2025"
  if (sscanf(str, "%s %s %d %d:%d:%d %d", week, month, &day, &hour, &min, &sec,
             &year) == 7) {
    out->date.day = (uint8_t)day;
    out->date.year = (uint8_t)(year % 100);
    out->time.hour = (uint8_t)hour;
    out->time.minute = (uint8_t)min;
    out->time.second = (uint8_t)sec;

    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    for (int i = 0; i < 12; i++) {
      if (strcmp(month, months[i]) == 0) {
        out->date.month = i + 1;
        break;
      }
    }
    const char *weeks[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    for (int i = 0; i < 7; i++) {
      if (strcmp(week, weeks[i]) == 0) {
        out->date.week = i + 1;
        break;
      }
    }
  }
}

static void parse_sntp_time(void *ctx, const char *line) {
  esp8266_sntp_driver_t *self = (esp8266_sntp_driver_t *)ctx;
  if (strncmp(line, "+CIPSNTPTIME:", 13) == 0) {
    strncpy(self->time_buf, line + 13, sizeof(self->time_buf) - 1);
    self->time_buf[sizeof(self->time_buf) - 1] = '\0';
  }
}

static void on_sync_response(void *ctx, AT_CmdResult_t result,
                             const char *last_line) {
  esp8266_sntp_driver_t *self = (esp8266_sntp_driver_t *)ctx;
  if (!self || !self->pending_cb)
    return;

  int res = -1;
  sntp_time_t time_data = {0};

  if (result == AT_CMD_OK && self->time_buf[0] != '\0') {
    parse_sntp_to_rtc(self->time_buf, &time_data);
    if (time_data.date.year != 0) { // 简单校验
      res = 0;
      self->status = SNTP_DRV_STATUS_SUCCESS;
    } else {
      self->status = SNTP_DRV_STATUS_FAIL;
    }
  } else {
    self->status = SNTP_DRV_STATUS_FAIL;
  }

  self->pending_cb(self->pending_user_data, res, &time_data);
  self->pending_cb = NULL;
}

static void on_config_response(void *ctx, AT_CmdResult_t result,
                               const char *last_line) {
  esp8266_sntp_driver_t *self = (esp8266_sntp_driver_t *)ctx;
  if (!self)
    return;
  if (result == AT_CMD_OK) {
    log_i("SNTP Configured.");
    self->status = SNTP_DRV_STATUS_PENDING;
  } else {
    self->status = SNTP_DRV_STATUS_FAIL;
  }
}

void esp8266_sntp_driver_init(esp8266_sntp_driver_t *self,
                              at_controller_t *at) {
  if (!self || !at)
    return;
  memset(self, 0, sizeof(esp8266_sntp_driver_t));
  self->base.ops = &esp8266_sntp_ops;
  self->at = at;
  self->status = SNTP_DRV_STATUS_IDLE;

  at_controller_register_handler(at, "+TIME_UPDATED", handle_time_updated,
                                 self);
}

static int esp8266_sntp_config(sntp_driver_t *base, int timezone,
                               const char *server1) {
  esp8266_sntp_driver_t *self = (esp8266_sntp_driver_t *)base;
  char cmd[128];
  snprintf(cmd, sizeof(cmd), "AT+CIPSNTPCFG=1,%d,\"%s\"\r\n", timezone,
           server1);

  AT_Cmd_t at_cmd = {
      .response_cb = on_config_response,
      .cmd_str = cmd,
      .timeout_ms = 2000,
      .ctx = self,
  };
  return at_controller_submit_cmd(self->at, &at_cmd);
}

static int esp8266_sntp_start_sync(sntp_driver_t *base, sntp_sync_cb_t cb,
                                   void *user_data) {
  esp8266_sntp_driver_t *self = (esp8266_sntp_driver_t *)base;
  if (SNTP_DRV_STATUS_SUCCESS != self->status) {
    log_e("SNTP not ready");
    return -1;
  }

  self->pending_cb = cb;
  self->pending_user_data = user_data;
  self->time_buf[0] = '\0';

  AT_Cmd_t at_cmd = {
      .cmd_str = "AT+CIPSNTPTIME?\r\n",
      .timeout_ms = 3000,
      .parser_cb = parse_sntp_time,
      .response_cb = on_sync_response,
      .ctx = self,
  };
  return at_controller_submit_cmd(self->at, &at_cmd);
}

static sntp_drv_status_t esp8266_sntp_get_status(sntp_driver_t *base) {
  return ((esp8266_sntp_driver_t *)base)->status;
}
