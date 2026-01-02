#include "mqtt_service.h"
#include <stdio.h>
#include <string.h>


#define LOG_TAG "MQTT_SVC"
#include "elog.h"

// --- Local Event Handler for Driver Events ---
static void mqtt_drv_event_handler(void *arg, mqtt_drv_event_t *event) {
  mqtt_service_t *self = (mqtt_service_t *)arg;

  switch (event->type) {
  case MQTT_DRV_EVENT_CONNECTED:
    self->state = MQTT_SVC_STATE_CONNECTED;
    log_i("Service: MQTT Connected.");
    // Here we could auto-subscribe to topics if needed
    break;

  case MQTT_DRV_EVENT_DISCONNECTED:
    self->state = MQTT_SVC_STATE_DISCONNECTED;
    log_w("Service: MQTT Disconnected.");
    break;

  case MQTT_DRV_EVENT_DATA: {
    log_i("Service: Data received on topic %s", event->topic);

    char prop_id[32];
    thing_value_t val;
    char msg_id[32];

    if (self->adapter && self->adapter->parse_command(event->payload, prop_id,
                                                      &val, msg_id) == 0) {
      // Find device and update property
      // For simplified demo, we assume the first device or derive device from
      // topic thing_model_set_prop("TBD", prop_id, val, 1 /* Source: Cloud */);

      // Cloud Reply (Safe & Generic)
      char reply_topic[128];
      char reply_payload[256];
      self->adapter->get_reply_topic("TBD", reply_topic, sizeof(reply_topic));
      self->adapter->get_reply_payload(msg_id, 200, reply_payload,
                                       sizeof(reply_payload));

      MQTT_DRV_PUBLISH(self->drv, reply_topic, reply_payload, 0);
    }
    break;
  }
  }
}

void mqtt_svc_init(mqtt_service_t *self, mqtt_driver_t *drv,
                   const mqtt_adapter_t *adapter) {
  memset(self, 0, sizeof(mqtt_service_t));
  self->drv = drv;
  self->adapter = adapter;
  self->state = MQTT_SVC_STATE_DISCONNECTED;

  // Wire up the driver events to the service
  if (self->drv) {
    MQTT_DRV_SET_CB(self->drv, mqtt_drv_event_handler, self);
  }
}

int mqtt_svc_connect(mqtt_service_t *self) {
  if (!self->drv || !self->adapter)
    return -1;

  mqtt_conn_params_t params;
  self->adapter->get_conn_params(&params);

  mqtt_driver_conn_params_t drv_params = {.host = params.host,
                                          .port = params.port,
                                          .client_id = params.client_id,
                                          .username = params.username,
                                          .password = params.password,
                                          .keepalive = 120};

  self->state = MQTT_SVC_STATE_CONNECTING;
  return MQTT_DRV_CONNECT(self->drv, &drv_params);
}

int mqtt_svc_disconnect(mqtt_service_t *self) {
  if (!self->drv)
    return -1;
  return MQTT_DRV_DISCONNECT(self->drv);
}

void mqtt_svc_process(mqtt_service_t *self) {
  // Reconnection logic could be added here
}

int mqtt_svc_publish_property(mqtt_service_t *self,
                              const thing_device_t *device,
                              const thing_property_t *prop) {
  if (self->state != MQTT_SVC_STATE_CONNECTED || !self->drv || !self->adapter)
    return -1;

  char topic[128];
  char payload[256];
  self->adapter->get_post_topic(device->device_id, topic, sizeof(topic));
  self->adapter->serialize_post(device, prop, payload, sizeof(payload));

  return MQTT_DRV_PUBLISH(self->drv, topic, payload, 0);
}

mqtt_svc_state_t mqtt_svc_get_state(mqtt_service_t *self) {
  return self->state;
}
