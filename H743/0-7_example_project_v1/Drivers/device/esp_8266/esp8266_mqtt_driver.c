#include "esp8266_mqtt_driver.h"
#include <stdio.h>
#include <string.h>

#define LOG_TAG "ESP_MQTT_DRV"
#include "elog.h"

/****************
 * 内部URC处理
 ****************/
static void handle_connected(void *ctx, const char *line) {
  esp8266_mqtt_driver_t *self = (esp8266_mqtt_driver_t *)ctx;
  self->is_connected = true;
  if (self->event_cb) {
    mqtt_drv_event_t evt = {.type = MQTT_DRV_EVENT_CONNECTED};
    self->event_cb(self->event_arg, &evt); // 连接事件通知
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

/**
 * @brief 处理MQTT订阅接收数据
 */
static void handle_recv(void *ctx, const char *line) {
  esp8266_mqtt_driver_t *self = (esp8266_mqtt_driver_t *)ctx;
  if (!self->event_cb)
    return;

  // Line format: +MQTTSUBRECV:0,"topic",length,payload
  char topic[128];
  int len;
  char payload_start[256];
  // %[^\"] : 匹配所有不是双引号的字符，直到遇到第一个双引号为止
  if (sscanf(line, "+MQTTSUBRECV:0,\"%[^\"]\",%d,%s", topic, &len,
             payload_start) >= 2) {
    // 构造事件并回调
    mqtt_drv_event_t evt = {.type = MQTT_DRV_EVENT_DATA,
                            .topic = topic,
                            .payload = payload_start,
                            .payload_len = (uint16_t)len};
    self->event_cb(self->event_arg, &evt);
  }
}

/****************
 * 驱动接口实现
 ****************/
/**
 * @brief 连接到MQTT服务器
 * @param base 驱动实例
 * @param params 连接参数
 */
static int drv_connect(mqtt_driver_t *base,
                       const mqtt_driver_conn_params_t *params) {
  esp8266_mqtt_driver_t *self = (esp8266_mqtt_driver_t *)base;
  char cmd[256];
  // 如果是重连需先清理资源(模块要求),第一次执行,此条命令会失败,不影响后续连接
  snprintf(cmd, sizeof(cmd), "AT+MQTTCLEAN=0\r\n");
  AT_Cmd_t clean_cmd = {.cmd_str = cmd, .timeout_ms = 5000};
  if (at_controller_submit_cmd(self->at_ctrl, &clean_cmd)) {
    log_e("Failed to clean previous MQTT session");
    return -1;
  }

  // 1. 设置用户信息
  snprintf(cmd, sizeof(cmd),
           "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",
           params->client_id, params->username, "");
  AT_Cmd_t user_cmd = {.cmd_str = cmd, .timeout_ms = 5000};
  if (at_controller_submit_cmd(self->at_ctrl, &user_cmd)) {
    log_e("Failed to set MQTT user info");
    return -1;
  }

  // 2. 密码使用长格式发送(SERCFG长度限制)
  snprintf(cmd, sizeof(cmd), "AT+MQTTLONGPASSWORD=0,%d\r\n",
           strlen(params->password));
  AT_Cmd_t pwd_cmd = {
      .cmd_str = cmd, .data_to_send = params->password, .timeout_ms = 5000};
  if (at_controller_submit_cmd(self->at_ctrl, &pwd_cmd)) {
    log_e("Failed to set MQTT password");
    return -1;
  }

  // 3. Connect
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

/**
 * @brief 发布数据
 * @param base 驱动实例
 * @param topic 发布的Topic
 * @param payload 发布的数据
 * @param qos QOS等级
 */
static int drv_publish(mqtt_driver_t *base, const char *topic,
                       const char *payload, int qos) {
  esp8266_mqtt_driver_t *self = (esp8266_mqtt_driver_t *)base;
  char cmd[256];
  uint32_t payload_len = (uint32_t)strlen(payload);
  snprintf(cmd, sizeof(cmd), "AT+MQTTPUBRAW=0,\"%s\",%d,%d,0\r\n", topic,
           payload_len, qos);
  AT_Cmd_t pub_cmd = {
      .cmd_str = cmd, .data_to_send = payload, .timeout_ms = 5000};
  return at_controller_submit_cmd(self->at_ctrl, &pub_cmd);
}

/**
 * @brief 订阅Topic
 * @param base 驱动实例
 * @param topic 订阅的Topic
 * @param qos QOS等级
 */
static int drv_subscribe(mqtt_driver_t *base, const char *topic, int qos) {
  esp8266_mqtt_driver_t *self = (esp8266_mqtt_driver_t *)base;
  char cmd[256];
  snprintf(cmd, sizeof(cmd), "AT+MQTTSUB=0,\"%s\",%d\r\n", topic, qos);
  AT_Cmd_t sub_cmd = {.cmd_str = cmd, .timeout_ms = 5000};
  return at_controller_submit_cmd(self->at_ctrl, &sub_cmd);
}

/**
 * @brief 设置事件回调
 * @param base 驱动实例
 * @param cb 事件回调函数
 * @param arg 回调函数参数
 */
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

  // 注册内部的URC处理到AT控制器
  at_controller_register_handler(at_ctrl, "+MQTTCONNECTED", handle_connected,
                                 self);
  at_controller_register_handler(at_ctrl, "+MQTTDISCONNECTED",
                                 handle_disconnected, self);
  at_controller_register_handler(at_ctrl, "+MQTTSUBRECV", handle_recv, self);
}
