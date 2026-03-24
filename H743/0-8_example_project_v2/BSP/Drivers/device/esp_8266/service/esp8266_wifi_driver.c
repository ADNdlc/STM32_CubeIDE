/*
 * esp8266_wifi_driver.c
 *
 *  Implementation of the ESP8266 WiFi driver.
 */

#include "esp8266_wifi_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "esp8266_wifi"
#include "elog.h"

// --- Forward Declarations ---
static int esp8266_set_mode(wifi_driver_t *base, wifi_mode_t mode);
static int esp8266_connect(wifi_driver_t *base, const char *ssid,
                           const char *pwd);
static int esp8266_disconnect(wifi_driver_t *base);
static int esp8266_scan(wifi_driver_t *base, wifi_scan_cb_t cb, void *arg);
static wifi_status_t esp8266_get_status(wifi_driver_t *base);

static const wifi_driver_ops_t esp8266_ops = {
    .set_mode = esp8266_set_mode,
    .connect = esp8266_connect,
    .disconnect = esp8266_disconnect,
    .scan = esp8266_scan,
    .get_status = esp8266_get_status,
};

// --- URC 处理回调 ---
static void handle_wifi_connected(void *ctx, const char *line) {
  esp8266_wifi_driver_t *self = (esp8266_wifi_driver_t *)ctx;
  self->status = WIFI_STATUS_CONNECTED;
  log_i("WiFi Connected");
}

static void handle_wifi_got_ip(void *ctx, const char *line) {
  esp8266_wifi_driver_t *self = (esp8266_wifi_driver_t *)ctx;
  self->status = WIFI_STATUS_GOT_IP;
  log_i("WiFi Got IP");
}

static void handle_wifi_disconnected(void *ctx, const char *line) {
  esp8266_wifi_driver_t *self = (esp8266_wifi_driver_t *)ctx;
  self->status = WIFI_STATUS_DISCONNECTED;
  log_i("WiFi Disconnected");
}

// --- 驱动接口实现 ---

void esp8266_wifi_driver_init(esp8266_wifi_driver_t *self,
                              at_controller_t *at_ctrl) {
  if (!self || !at_ctrl)
    return;

  self->base.ops = &esp8266_ops;
  self->at_ctrl = at_ctrl;
  self->status = WIFI_STATUS_DISCONNECTED;
  self->mode = WIFI_MODE_STATION;

  // 初始化AP列表指针
  self->result_list.count = 0;
  self->result_list.ap_info = self->ap_buffer;

  // 注册URC处理
  at_controller_register_handler(at_ctrl, "WIFI CONNECTED",
                                 handle_wifi_connected, self);
  at_controller_register_handler(at_ctrl, "WIFI GOT IP", handle_wifi_got_ip,
                                 self);
  at_controller_register_handler(at_ctrl, "WIFI DISCONNECT",
                                 handle_wifi_disconnected, self);
}

/**
 * @brief 设置WiFi工作模式
 *
 * @param base 驱动实例
 * @param mode 工作模式
 * @return int 0:提交成功, -1:提交失败
 */
static int esp8266_set_mode(wifi_driver_t *base, wifi_mode_t mode) {
  esp8266_wifi_driver_t *self = (esp8266_wifi_driver_t *)base;
  char cmd[32];
  // AT+CWMODE=<mode>[,<auto_connect>] - auto_connect=0 to disable auto
  // reconnect on mode switch
  snprintf(cmd, sizeof(cmd), "AT+CWMODE=%d,0\r\n", (int)mode);

  AT_Cmd_t at_cmd = {
      .cmd_str = cmd,
      .timeout_ms = 1000,
  };

  if (at_controller_submit_cmd(self->at_ctrl, &at_cmd) == 0) {
    self->mode = mode;
    return 0;
  }
  return -1;
}

/**
 * @brief 连接WiFi网络
 *
 * @param base 驱动实例
 * @param ssid 连接ap的SSID
 * @param pwd  连接ap的密码
 * @return int 0:提交成功, -1:提交失败
 */
static int esp8266_connect(wifi_driver_t *base, const char *ssid,
                           const char *pwd) {
  esp8266_wifi_driver_t *self = (esp8266_wifi_driver_t *)base;
  char cmd[128];
  snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwd);

  AT_Cmd_t at_cmd = {
      .cmd_str = cmd,
      .timeout_ms = 20000, // Connecting can take a long time
  };

  self->status = WIFI_STATUS_CONNECTING; // 状态变为连接中
  return at_controller_submit_cmd(self->at_ctrl, &at_cmd);
}

/**
 * @brief 断开WiFi连接
 */
static int esp8266_disconnect(wifi_driver_t *base) {
  esp8266_wifi_driver_t *self = (esp8266_wifi_driver_t *)base;
  AT_Cmd_t at_cmd = {
      .cmd_str = "AT+CWQAP\r\n",
      .timeout_ms = 2000,
  };
  return at_controller_submit_cmd(self->at_ctrl, &at_cmd);
}

/**
 * @brief AP扫描结果解析(解析一条)
 * 扫描结果格式:
 * +CWLAP:(<ecn>,<"ssid">,<rssi>,<"mac">,<channel>,<freq_offset>,<freqcal_val>,<pairwise_cipher>,<group_cipher>,<wifi_protocol>,<wps>)
 */
static void parse_scan_result(void *ctx, const char *line) {
  esp8266_wifi_driver_t *self = (esp8266_wifi_driver_t *)ctx;
  if (!self || strncmp(line, "+CWLAP:", 7) != 0) // 检查前缀
    return;

  if (self->result_list.count >= 10)
    return;

  wifi_ap_info_t *ap =
      &self->result_list.ap_info[self->result_list.count]; // 使用实例自带缓冲区

  // +CWLAP:(3,"test1",-20,"f1:f2:f3:f4:f5:67",6,-1,-1,4,4,7,0)
  int ecn, rssi, channel, pw_cipher, gr_cipher, proto, wps;

  const char *p = line + 8; // skip "+CWLAP:("

  // <ecn>
  ecn = atoi(p);      // 转换为整数
  p = strchr(p, ','); // 找到下一项(","为分隔符)
  if (!p)
    return;
  p++; // 现在应该指向"SSID"开头的双引号

  // <"ssid">
  if (*p == '\"')
    p++;
  const char *ssid_end = strchr(p, '\"'); // 找到"SSID"结束的双引号
  if (!ssid_end)
    return;

  size_t ssid_len = ssid_end - p; // 计算SSID长度
  if (ssid_len >= sizeof(ap->ssid))
    ssid_len = sizeof(ap->ssid) - 1;
  memcpy(ap->ssid, p, ssid_len); // 复制SSID内容
  ap->ssid[ssid_len] = '\0';
  p = ssid_end + 2; // 跳过引号和逗号

  // <rssi>
  rssi = atoi(p);
  p = strchr(p, ',');
  if (!p)
    return;
  p++;

  // <"mac">
  if (*p == '\"')
    p++;
  const char *mac_end = strchr(p, '\"');
  if (!mac_end)
    return;

  size_t mac_len = mac_end - p;
  if (mac_len >= sizeof(ap->mac))
    mac_len = sizeof(ap->mac) - 1;
  memcpy(ap->mac, p, mac_len);
  ap->mac[mac_len] = '\0';
  p = mac_end + 2; // skip quote and comma

  // <channel>
  channel = atoi(p);
  p = strchr(p, ',');
  if (!p)
    return;
  p++;

  // skip <freq_offset>, <freqcal_val>
  p = strchr(p, ',');
  if (!p)
    return;
  p++;
  p = strchr(p, ',');
  if (!p)
    return;
  p++; // 应该指向pairwise_cipher值

  // <pairwise_cipher>
  pw_cipher = atoi(p);
  p = strchr(p, ',');
  if (!p)
    return;
  p++;

  // <group_cipher>
  gr_cipher = atoi(p);
  p = strchr(p, ',');
  if (!p)
    return;
  p++;

  // <wifi_protocol>
  proto = atoi(p);
  p = strchr(p, ',');
  if (!p)
    return;
  p++;

  // <wps>
  wps = atoi(p);

  ap->encryption = (uint8_t)ecn;
  ap->rssi = (int8_t)rssi;
  ap->channel = (uint8_t)channel;
  ap->pairwise_cipher = (uint8_t)pw_cipher;
  ap->group_cipher = (uint8_t)gr_cipher;
  ap->protocol = (uint8_t)proto;
  ap->wps = (uint8_t)wps;

  self->result_list.count++;
  log_d("Parsed AP: %s (RSSI: %d)", ap->ssid, ap->rssi);
}

static void scan_finish_cb_t(void *ctx, AT_CmdResult_t result,
                             const char *last_line) {
  esp8266_wifi_driver_t *self = (esp8266_wifi_driver_t *)ctx;
  if (!self)
    return;

  if (self->scan_cb) {
    // 从上下文中找到传入的回调,
    self->scan_cb(self->scan_arg, &(self->result_list));
    // 调用后清除回调，防止重复调用（如果需要的话，通常扫描是一次性的）
    self->scan_cb = NULL;
    self->scan_arg = NULL;
  }
}

static int esp8266_scan(wifi_driver_t *base, wifi_scan_cb_t cb, void *arg) {
  esp8266_wifi_driver_t *self = (esp8266_wifi_driver_t *)base;
  self->result_list.count = 0; // Reset count
  self->scan_cb = cb;          // Store callback
  self->scan_arg = arg;        // Store argument

  AT_Cmd_t at_cmd = {
      .cmd_str = "AT+CWLAP\r\n",
      .timeout_ms = 10000,
      .parser_cb =
          (at_parser_cb_t)parse_scan_result, // 解析函数设置为查询处理回调
      .response_cb = (at_response_cb_t)scan_finish_cb_t,
      .ctx = self, // 将self传入作为上下文，以便在回调中获取scan_results和cb
  };

  return at_controller_submit_cmd(self->at_ctrl, &at_cmd);
}

static wifi_status_t esp8266_get_status(wifi_driver_t *base) {
  esp8266_wifi_driver_t *self = (esp8266_wifi_driver_t *)base;
  return self->status;
}
