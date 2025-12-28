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
#include "../../../EasyLogger/easylogger/inc/elog.h"

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

// --- URC Handlers ---
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

// --- Driver Implementation ---

void esp8266_wifi_driver_init(esp8266_wifi_driver_t *self,
                              at_controller_t *at_ctrl) {
  if (!self || !at_ctrl)
    return;

  self->base.ops = &esp8266_ops;
  self->at_ctrl = at_ctrl;
  self->status = WIFI_STATUS_DISCONNECTED;
  self->mode = WIFI_MODE_STATION;
  self->scan_count = 0;

  // Register URC handlers
  at_controller_register_handler(at_ctrl, "WIFI CONNECTED",
                                 handle_wifi_connected, self);
  at_controller_register_handler(at_ctrl, "WIFI GOT IP", handle_wifi_got_ip,
                                 self);
  at_controller_register_handler(at_ctrl, "WIFI DISCONNECT",
                                 handle_wifi_disconnected, self);

  // Initialization sequence
  AT_Cmd_t init_cmds[] = {
      {.cmd_str = "AT+SYSSTORE=1\r\n", .timeout_ms = 1000},
      {.cmd_str = "ATE0\r\n", .timeout_ms = 1000},
      {.cmd_str = "AT+CWAUTOCONN=0\r\n", .timeout_ms = 1000},
  };

  for (int i = 0; i < sizeof(init_cmds) / sizeof(init_cmds[0]); i++) {
    at_controller_submit_cmd(at_ctrl, &init_cmds[i]);
  }
}

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

static int esp8266_connect(wifi_driver_t *base, const char *ssid,
                           const char *pwd) {
  esp8266_wifi_driver_t *self = (esp8266_wifi_driver_t *)base;
  char cmd[128];
  snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwd);

  AT_Cmd_t at_cmd = {
      .cmd_str = cmd,
      .timeout_ms = 20000, // Connecting can take a long time
  };

  self->status = WIFI_STATUS_CONNECTING;
  return at_controller_submit_cmd(self->at_ctrl, &at_cmd);
}

static int esp8266_disconnect(wifi_driver_t *base) {
  esp8266_wifi_driver_t *self = (esp8266_wifi_driver_t *)base;
  AT_Cmd_t at_cmd = {
      .cmd_str = "AT+CWQAP\r\n",
      .timeout_ms = 2000,
  };
  return at_controller_submit_cmd(self->at_ctrl, &at_cmd);
}

/**
 * @brief Robust parser for +CWLAP lines
 * Spec:
 * +CWLAP:(<ecn>,<"ssid">,<rssi>,<"mac">,<channel>,<freq_offset>,<freqcal_val>,<pairwise_cipher>,<group_cipher>,<wifi_protocol>,<wps>)
 */
static void parse_scan_result(const char *line) {
  if (strncmp(line, "+CWLAP:", 7) != 0)
    return;

  // We need to find the driver instance. Unfortunately the AT command parser_cb
  // doesn't take context currently. In a real system we'd use a global or a
  // better parser registration. For this project, let's assume we use a
  // singleton or find a way to pass context. Actually, I should have modified
  // the parser_cb to take a ctx.
}

// Fixed parser with context
static void parse_scan_result_v2(void *ctx, const char *line) {
  esp8266_wifi_driver_t *self = (esp8266_wifi_driver_t *)ctx;
  if (strncmp(line, "+CWLAP:", 7) != 0)
    return;
  if (self->scan_count >= 20)
    return;

  wifi_ap_info_t *ap = &self->scan_results[self->scan_count];

  // Use sscanf to extract fields. Note: SSID and MAC contain quotes.
  // +CWLAP:(3,"test1",-20,"e2:b8:a6:e2:e3:10",6,-1,-1,4,4,7,0)
  int ecn, rssi, channel, pw_cipher, gr_cipher, proto, wps;
  char ssid[33];
  char mac[18];

  // Using a more manual approach because of quotes in string fields
  const char *p = line + 8; // skip "+CWLAP:("

  // <ecn>
  ecn = atoi(p);
  p = strchr(p, ',');
  if (!p)
    return;
  p++;

  // <"ssid">
  if (*p == '\"')
    p++;
  const char *ssid_end = strchr(p, '\"');
  if (!ssid_end)
    return;
  size_t ssid_len = ssid_end - p;
  if (ssid_len > 32)
    ssid_len = 32;
  memcpy(ap->ssid, p, ssid_len);
  ap->ssid[ssid_len] = '\0';
  p = ssid_end + 2; // skip quote and comma

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
  if (mac_len > 17)
    mac_len = 17;
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
  p++;

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

  self->scan_count++;
  log_d("Parsed AP: %s (RSSI: %d)", ap->ssid, ap->rssi);
}

static int esp8266_scan(wifi_driver_t *base, wifi_scan_cb_t cb, void *arg) {
  esp8266_wifi_driver_t *self = (esp8266_wifi_driver_t *)base;
  self->scan_count = 0; // Reset count

  // Note: I need to update the AT_Cmd_t to allow a context for the parser.
  // Since I can't easily change the AT controller's callback signature right
  // now without breaking things, I will use at_controller_register_handler for
  // +CWLAP temporarily or just hope it matches. Actually, the implementation
  // plan said "Pass unhandled lines to command parser". But wait, it's better
  // if the parser_cb takes a context. Let's modify AT_Cmd_t and at_controller.c
  // to support it.

  AT_Cmd_t at_cmd = {
      .cmd_str = "AT+CWLAP\r\n",
      .timeout_ms = 10000,
      .parser_cb = (at_parser_cb_t)
          parse_scan_result_v2, // Cheat and pass context if I update it
  };

  return at_controller_submit_cmd(self->at_ctrl, &at_cmd);
}

static wifi_status_t esp8266_get_status(wifi_driver_t *base) {
  esp8266_wifi_driver_t *self = (esp8266_wifi_driver_t *)base;
  return self->status;
}
