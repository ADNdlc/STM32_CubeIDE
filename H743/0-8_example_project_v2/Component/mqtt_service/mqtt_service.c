#include "mqtt_service.h"
#include "mqtt_factory.h"
#include <stdio.h>
#include <string.h>

#define LOG_TAG "MQTT_SVC"
#include "elog.h"

/*******************
 * Internal Helpers
 *******************/
// 事件处理(遍历观察者列表)
static void notify_event(mqtt_service_t *self, const mqtt_drv_event_t *event) {
  for (uint8_t i = 0; i < self->observer_count; i++) {
    if (self->observers[i].cb) {
      self->observers[i].cb(self, event, self->observers[i].user_data);
    }
  }
}

// 服务层事件处理回调
static void mqtt_drv_event_handler(void *arg, mqtt_drv_event_t *event) {
  mqtt_service_t *self = (mqtt_service_t *)arg;
  if (!self)
    return;

  switch (event->type) {
  case MQTT_DRV_EVENT_CONNECTED:
    self->state = MQTT_SVC_STATE_CONNECTED;
    self->retry_count = 0;
    log_i("Service: MQTT Connected.");
    break;
  case MQTT_DRV_EVENT_DISCONNECTED:
    self->state = MQTT_SVC_STATE_DISCONNECTED;
    log_w("Service: MQTT Disconnected.");
    break;
  case MQTT_DRV_EVENT_DATA:
    log_d("Service: Data received on topic %s", event->topic);
    break;
  default:
    break;
  }

  notify_event(self, event); // 传递事件给各个观察者
}

/*******************
 * Public APIs
 *******************/

void mqtt_svc_init(mqtt_service_t *self, mqtt_driver_t *drv,
                   const mqtt_adapter_t *adapter) {
  memset(self, 0, sizeof(mqtt_service_t));
  self->drv = drv;
  self->adapter = adapter;
  self->state = MQTT_SVC_STATE_DISCONNECTED;
  self->observer_count = 0;

  if (self->drv) {
    MQTT_DRV_SET_CB(self->drv, mqtt_drv_event_handler,
                    self); // 服务层回调对接到驱动层
  }
}

int mqtt_svc_register_callback(mqtt_service_t *self, mqtt_svc_event_cb_t cb,
                               void *user_data) {
  if (self->observer_count >= MAX_MQTT_SVC_OBSERVERS) {
    log_e("MQTT SVC: Observer list full!");
    return -1;
  }
  self->observers[self->observer_count].cb = cb;
  self->observers[self->observer_count].user_data = user_data;
  self->observer_count++;
  return 0;
}
#include "elog.h"
int mqtt_svc_connect(mqtt_service_t *self) {
  if (!self->drv){
	log_e("mqtt_svc driver NULL");
	return -1;
  }
  if (!self->adapter){
	log_e("mqtt_svc adapter NULL");
	return -1;
  }

  mqtt_conn_params_t params;
  self->adapter->get_conn_params(&params); // 获取平台连接参数
  log_v("MQTT Service: Connecting to %s:%d", params.host, params.port);
  log_v("MQTT Service: Client ID: %s", params.client_id);
  log_v("MQTT Service: Username: %s", params.username);
  log_v("MQTT Service: Password: %s", params.password);
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

int mqtt_svc_subscribe(mqtt_service_t *self, const char *topic, uint8_t qos) {
  if (!self->drv)
    return -1;
  return MQTT_DRV_SUBSCRIBE(self->drv, topic, qos);
}

int mqtt_svc_publish_property(mqtt_service_t *self,
                              const thing_device_t *device,
                              const thing_property_t *prop) {
  if (self->state != MQTT_SVC_STATE_CONNECTED || !self->drv || !self->adapter)
    return -1;

  char topic[128];
  char buf[256];
  // 通过适配器获取发布主题和数据
  if (self->adapter &&
      self->adapter->serialize_post(device, prop, buf, sizeof(buf)) == 0) {
    self->adapter->get_topic(device->device_id, MQTT_TOPIC_PROPERTY_POST, topic,
                             sizeof(topic));
    return MQTT_DRV_PUBLISH(self->drv, topic, buf, 0);
  }
  return -1;
}

void mqtt_svc_reply_command(mqtt_service_t *self, const char *device_id,
                            const char *msg_id, int code) {
  if (!self || !self->adapter || !self->drv)
    return;

  char reply_topic[128];
  char reply_payload[128];
  self->adapter->get_topic(device_id, MQTT_TOPIC_PROPERTY_SET_REPLY,
                           reply_topic, sizeof(reply_topic));
  self->adapter->get_reply_payload(msg_id, code, reply_payload,
                                   sizeof(reply_payload));
  MQTT_DRV_PUBLISH(self->drv, reply_topic, reply_payload, 0);
}

mqtt_svc_state_t mqtt_svc_get_state(mqtt_service_t *self) {
  return self->state;
}

void mqtt_svc_process(mqtt_service_t *self) {
  // Transport layer processing if needed
}
