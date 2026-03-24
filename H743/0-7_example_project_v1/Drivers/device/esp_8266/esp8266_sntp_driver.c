/*
 * esp8266_sntp_driver.c
 *
 *  Implementation of the ESP8266 SNTP driver.
 */

#include "esp8266_sntp_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "esp8266_sntp"
#include "elog.h"

// --- Forward Declarations ---
static int esp8266_sntp_config(sntp_driver_t *base, int enable, int timezone,
                               const char *server1);
static int esp8266_sntp_get_time(sntp_driver_t *base, sntp_get_time_cb_t cb,
                                 void *user_data);
static sntp_drv_status_t esp8266_sntp_get_status(sntp_driver_t *base);

static const sntp_driver_ops_t esp8266_sntp_ops = {
    .config = esp8266_sntp_config,
    .get_time = esp8266_sntp_get_time,
    .get_status = esp8266_sntp_get_status,
};

// --- URC 处理 ---
// +TIME_UPDATED
static void handle_time_updated(void *ctx, const char *line) {
  esp8266_sntp_driver_t *self = (esp8266_sntp_driver_t *)ctx;
  self->status = SNTP_DRV_STATUS_SYNCED;  // 已同步,可获取时间
  log_i("SNTP Status: Synchronized (URC received)");
}

// 解析时间内容: +CIPSNTPTIME:Fri May 30 18:06:18 2025
static void parse_sntp_time(void *ctx, const char *line) {
  esp8266_sntp_driver_t *self = (esp8266_sntp_driver_t *)ctx;
  if (strncmp(line, "+CIPSNTPTIME:", 13) == 0) {
    // 将模块返回数据解析并存入缓冲区
    strncpy(self->time_buf, line + 13, sizeof(self->time_buf) - 1);
    self->time_buf[sizeof(self->time_buf) - 1] = '\0';
  }
}

// 命令结果回调 (Now with ctx support)
static void on_get_time_response(void *ctx, AT_CmdResult_t result,
                                 const char *last_line) {
  esp8266_sntp_driver_t *self = (esp8266_sntp_driver_t *)ctx;
  if (!self)
    return;

  if (self->pending_cb) {
    int res = (result == AT_CMD_OK) ? 0 : -1;
    // 将时间传给服务层回调
    self->pending_cb(&self->base, res, self->time_buf, self->pending_user_data);
    self->pending_cb = NULL;
  }
}

static void on_congfig_response(void *ctx, AT_CmdResult_t result,
                                const char *last_line) {
  esp8266_sntp_driver_t *self = (esp8266_sntp_driver_t *)ctx;
  if (!self)
    return;

  if (result == AT_CMD_OK) {
    log_i("SNTP Status: Configured");
    self->status = SNTP_DRV_STATUS_WAITING; //新的服务器设置成功,等待模块同步(+TIME_UPDATED)
  }
}

void esp8266_sntp_driver_init(esp8266_sntp_driver_t *self,
                              at_controller_t *at) {
  if (!self || !at)
    return;
  memset(self, 0, sizeof(esp8266_sntp_driver_t));
  self->base.ops = &esp8266_sntp_ops;
  self->at = at;
  self->status = SNTP_DRV_TATUS_NOCONFIGS;

  // 注册 URC
  at_controller_register_handler(at, "+TIME_UPDATED", handle_time_updated, self);
}

static int esp8266_sntp_config(sntp_driver_t *base, int enable, int timezone,
                               const char *server1) {
  esp8266_sntp_driver_t *self = (esp8266_sntp_driver_t *)base;
  char cmd[128];
  // AT+CIPSNTPCFG=<enable>,<timezone>,<"server1">,<"server2">,<"server3">
  snprintf(cmd, sizeof(cmd), "AT+CIPSNTPCFG=%d,%d,\"%s\"\r\n", enable, timezone,
           server1);

  AT_Cmd_t at_cmd = {
      .response_cb = on_congfig_response,
      .cmd_str = cmd,
      .timeout_ms = 2000,
  };
  self->status = SNTP_DRV_TATUS_NOCONFIGS;  // 配置新的服务器
  at_controller_submit_cmd(self->at, &at_cmd);
  return 0;
}

static int esp8266_sntp_get_time(sntp_driver_t *base, sntp_get_time_cb_t cb,
                                 void *user_data) {
  esp8266_sntp_driver_t *self = (esp8266_sntp_driver_t *)base;
  if(self->status == SNTP_DRV_TATUS_NOCONFIGS) {
    log_e("SNTP Status: Not configured");
    return -1;
  }

  self->pending_cb = cb;  // 设置本次查询结果的回调
  self->pending_user_data = user_data;
  self->time_buf[0] = '\0'; // Clear previous data

  AT_Cmd_t at_cmd = {.cmd_str = "AT+CIPSNTPTIME?\r\n",
                     .timeout_ms = 2000,
                     .parser_cb = parse_sntp_time,        // 查询回调:解析时间
                     .response_cb = on_get_time_response, // 响应回调:处理结果
                     .ctx = self};

  return at_controller_submit_cmd(self->at, &at_cmd);
}

static sntp_drv_status_t esp8266_sntp_get_status(sntp_driver_t *base) {
  esp8266_sntp_driver_t *self = (esp8266_sntp_driver_t *)base;
  return self->status;
}
