#include "mqtt_service.h"
#include <stdio.h>
#include <string.h>


#define LOG_TAG "MQTT_SVC"
#include "elog.h"

// --- URC Handlers ---
static void handle_mqtt_connected(void *ctx, const char *line) {
  mqtt_service_t *self = (mqtt_service_t *)ctx;
  self->state = MQTT_SVC_STATE_CONNECTED;
  log_i("MQTT Connected to broker.");
}

static void handle_mqtt_disconnected(void *ctx, const char *line) {
  mqtt_service_t *self = (mqtt_service_t *)ctx;
  self->state = MQTT_SVC_STATE_DISCONNECTED;
  log_w("MQTT Disconnected.");
}

static void handle_mqtt_msg_recv(void *ctx, const char *line) {
  mqtt_service_t *self = (mqtt_service_t *)ctx;
  char prop_id[32];
  thing_value_t val;
  char msg_id[32];

  // Parse using platform adapter
  if (self->adapter &&
      self->adapter->parse_command(line, prop_id, &val, msg_id) == 0) {
    log_i("Received command: %s", prop_id);

    // Find device (assuming single device for now or getting from context)
    // In this architecture, net_mgr will coordinate this.
    // For now, we update the first device in thing_model or similar.
    // Better: adapter should handle which device it is based on topic.

    // thing_model_set_prop(..., prop_id, val, 1 /* Source: Cloud */);

    // Reply if needed
    char reply_topic[128];
    char reply_payload[256];
    self->adapter->get_reply_topic("TBD", reply_topic, sizeof(reply_topic));
    self->adapter->get_reply_payload(msg_id, 200, reply_payload,
                                     sizeof(reply_payload));

    // Submit publish command for reply
    // AT+MQTTPUB=0,"topic","payload",0,0
  }
}

void mqtt_svc_init(mqtt_service_t *self, at_controller_t *at_ctrl,
                   const mqtt_adapter_t *adapter) {
  memset(self, 0, sizeof(mqtt_service_t));
  self->at_ctrl = at_ctrl;
  self->adapter = adapter;
  self->state = MQTT_SVC_STATE_DISCONNECTED;

  // Register URCs
  at_controller_register_handler(at_ctrl, "+MQTTCONNECTED",
                                 handle_mqtt_connected, self);
  at_controller_register_handler(at_ctrl, "+MQTTDISCONNECTED",
                                 handle_mqtt_disconnected, self);
  at_controller_register_handler(at_ctrl, "+MQTTSUBRECV", handle_mqtt_msg_recv,
                                 self);
}

int mqtt_svc_connect(mqtt_service_t *self) {
  if (!self->adapter || !self->at_ctrl)
    return -1;

  mqtt_conn_params_t params;
  self->adapter->get_conn_params(&params);

  char cmd[512];

  // 1. Configure User
  // AT+MQTTUSERCFG=0,1,"client_id","user","pwd",0,0,""
  snprintf(cmd, sizeof(cmd),
           "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",
           params.client_id, params.username, params.password);
  AT_Cmd_t user_cmd = {.cmd_str = cmd, .timeout_ms = 5000};
  at_controller_submit_cmd(self->at_ctrl, &user_cmd);

  // 2. Connect
  // AT+MQTTCONN=0,"host",port,0
  snprintf(cmd, sizeof(cmd), "AT+MQTTCONN=0,\"%s\",%d,0\r\n", params.host,
           params.port);
  AT_Cmd_t conn_cmd = {.cmd_str = cmd, .timeout_ms = 10000};
  at_controller_submit_cmd(self->at_ctrl, &conn_cmd);

  self->state = MQTT_SVC_STATE_CONNECTING;
  return 0;
}

void mqtt_svc_process(mqtt_service_t *self) {
  // Simple reconnection logic could go here
}

int mqtt_svc_publish_property(mqtt_service_t *self,
                              const thing_device_t *device,
                              const thing_property_t *prop) {
  if (self->state != MQTT_SVC_STATE_CONNECTED || !self->adapter)
    return -1;

  char topic[128];
  char payload[256];
  self->adapter->get_post_topic(device->device_id, topic, sizeof(topic));
  self->adapter->serialize_post(device, prop, payload, sizeof(payload));

  char cmd[512];
  // AT+MQTTPUB=0,"topic","payload",0,0
  snprintf(cmd, sizeof(cmd), "AT+MQTTPUB=0,\"%s\",\"%s\",0,0\r\n", topic,
           payload);
  AT_Cmd_t pub_cmd = {.cmd_str = cmd, .timeout_ms = 5000};
  return at_controller_submit_cmd(self->at_ctrl, &pub_cmd);
}

mqtt_svc_state_t mqtt_svc_get_state(mqtt_service_t *self) {
  return self->state;
}
