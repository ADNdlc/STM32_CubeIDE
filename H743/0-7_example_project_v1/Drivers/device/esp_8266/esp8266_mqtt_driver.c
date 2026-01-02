#include "esp8266_mqtt_driver.h"
#include <stdio.h>
#include <string.h>


#define LOG_TAG "ESP_MQTT_DRV"
#include "elog.h"

// --- URC Handlers Forwarding to Driver Events ---
static void handle_connected(void *ctx, const char *line) {
  esp8266_mqtt_driver_t *self = (esp8266_mqtt_driver_t *)ctx;
  self->is_connected = true;
  if (self->event_cb) {
    mqtt_drv_event_t evt = {.type = MQTT_DRV_EVENT_CONNECTED};
    self->event_cb(self->event_arg, &evt);
  }
}

static void handle_disconnected(void *ctx, const char *line) {
  esp8266_mqtt_driver_t *self = (esp8266_mqtt_driver_t *)ctx;
  self->is_connected = false;
  if (self->event_cb) {
    mqtt_drv_event_t evt = {.type = MQTT_DRV_EVENT_DISCONNECTED};
    self->event_cb(self->event_arg, &evt);
  }
}

static void handle_recv(void *ctx, const char *line) {
  esp8266_mqtt_driver_t *self = (esp8266_mqtt_driver_t *)ctx;
  if (!self->event_cb)
    return;

  // Line format: +MQTTSUBRECV:0,"topic",length,payload
  // Note: Parsing needs to be robust.
  // This is a simplified regex-like scan.
  char topic[128];
  int len;
  char payload_start[256];

  if (sscanf(line, "+MQTTSUBRECV:0,\"%[^\"]\",%d,%s", topic, &len,
             payload_start) >= 2) {
    mqtt_drv_event_t evt = {.type = MQTT_DRV_EVENT_DATA,
                            .topic = topic,
                            .payload = payload_start,
                            .payload_len = (uint16_t)len};
    self->event_cb(self->event_arg, &evt);
  }
}

// --- Driver Operations ---

static int drv_connect(mqtt_driver_t *base,
                       const mqtt_driver_conn_params_t *params) {
  esp8266_mqtt_driver_t *self = (esp8266_mqtt_driver_t *)base;
  char cmd[256];
  // 重连需先清理资源(模块要求)
  snprintf(cmd, sizeof(cmd), "AT+MQTTCLEAN=0\r\n");
  AT_Cmd_t clean_cmd = {.cmd_str = cmd, .timeout_ms = 5000};
  if(at_controller_submit_cmd(self->at_ctrl, &clean_cmd)) {
    log_e("Failed to clean previous MQTT session");
    return -1;
  }

  // 1. 设置用户信息
  snprintf(cmd, sizeof(cmd),
           "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",
           params->client_id, params->username, "");
  AT_Cmd_t user_cmd = {.cmd_str = cmd, .timeout_ms = 5000};
  if(at_controller_submit_cmd(self->at_ctrl, &user_cmd)) {
    log_e("Failed to set MQTT user info");
    return -1;
  }

  // 密码使用长格式发送(SERCFG长度限制)
  snprintf(cmd, sizeof(cmd),
           "AT+MQTTLONGPASSWORD=0,\"%d\"\r\n", strlen(params->password));
  AT_Cmd_t pwd_cmd = {.cmd_str = cmd, .data_to_send = params->password, .timeout_ms = 5000};
  if(at_controller_submit_cmd(self->at_ctrl, &pwd_cmd)){
    log_e("Failed to set MQTT password");
    return -1;
  }
  
  // 2. Connect
  snprintf(cmd, sizeof(cmd), "AT+MQTTCONN=0,\"%s\",%d,0\r\n", params->host,
           params->port);
  AT_Cmd_t conn_cmd = {.cmd_str = cmd, .timeout_ms = 10000};
  return at_controller_submit_cmd(self->at_ctrl, &conn_cmd);
}

static int drv_disconnect(mqtt_driver_t *base) {
  esp8266_mqtt_driver_t *self = (esp8266_mqtt_driver_t *)base;
  char cmd[] = "AT+MQTTCLEAN=0\r\n";
  AT_Cmd_t clean_cmd = {.cmd_str = cmd, .timeout_ms = 5000};
  return at_controller_submit_cmd(self->at_ctrl, &clean_cmd);
}

static int drv_publish(mqtt_driver_t *base, const char *topic,
                       const char *payload, int qos) {
  esp8266_mqtt_driver_t *self = (esp8266_mqtt_driver_t *)base;
  char cmd[256];
  uint32_t payload_len = (uint32_t)strlen(payload);
  snprintf(cmd, sizeof(cmd), "AT+MQTTPUBRAW=0,\"%s\",\"%d\",%d,0\r\n", topic,
            payload_len, qos);
  AT_Cmd_t pub_cmd = {.cmd_str = cmd, .data_to_send = payload, .timeout_ms = 5000};
  return at_controller_submit_cmd(self->at_ctrl, &pub_cmd);
}

static int drv_subscribe(mqtt_driver_t *base, const char *topic, int qos) {
  esp8266_mqtt_driver_t *self = (esp8266_mqtt_driver_t *)base;
  char cmd[256];
  snprintf(cmd, sizeof(cmd), "AT+MQTTSUB=0,\"%s\",%d\r\n", topic, qos);
  AT_Cmd_t sub_cmd = {.cmd_str = cmd, .timeout_ms = 5000};
  return at_controller_submit_cmd(self->at_ctrl, &sub_cmd);
}

static void drv_set_event_cb(mqtt_driver_t *base, mqtt_drv_event_cb_t cb,
                             void *arg) {
  esp8266_mqtt_driver_t *self = (esp8266_mqtt_driver_t *)base;
  self->event_cb = cb;
  self->event_arg = arg;
}

static const mqtt_driver_ops_t g_esp8266_mqtt_ops = {
    .connect = drv_connect,
    .disconnect = drv_disconnect,
    .publish = drv_publish,
    .subscribe = drv_subscribe,
    .set_event_callback = drv_set_event_cb};

void esp8266_mqtt_driver_init(esp8266_mqtt_driver_t *self,
                              at_controller_t *at_ctrl) {
  memset(self, 0, sizeof(esp8266_mqtt_driver_t));
  self->base.ops = &g_esp8266_mqtt_ops;
  self->at_ctrl = at_ctrl;

  // Register URCs to the AT controller
  at_controller_register_handler(at_ctrl, "+MQTTCONNECTED", handle_connected,
                                 self);
  at_controller_register_handler(at_ctrl, "+MQTTDISCONNECTED",
                                 handle_disconnected, self);
  at_controller_register_handler(at_ctrl, "+MQTTSUBRECV", handle_recv, self);
}
