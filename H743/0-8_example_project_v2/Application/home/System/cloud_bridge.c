#include "cloud_bridge.h"
#include "mqtt_service/mqtt_adapter.h"
#include "mqtt_service/mqtt_service.h"
#include "sys.h"
#include <string.h>

#include "elog.h"
#define LOG_TAG "CLOUD_BR"

#define CLOUD_SYNC_INTERVAL_MS 1000 // 同步检查周期
// 内部状态和变量
static uint32_t g_last_sync_time = 0;
static mqtt_service_t *g_mqtt_svc = NULL;

// --- 解析属性回调 ---
static void on_prop_parsed(const char *prop_id, thing_value_t value,
                           void *ctx) {
  const char *device_id = (const char *)ctx;
  log_i("Applying property %s.%s from cloud", device_id, prop_id);
  // 云命令中解析出的属性值会通过此回调函数传递给物模型层
  thing_model_set_prop(device_id, prop_id, value,
                       THING_SOURCE_CLOUD); // 来源：云端
}

/**
 * @brief MQTT服务事件处理函数
 *
 * @param svc       MQTT服务实例
 * @param event     MQTT事件
 * @param user_data 用户数据
 */
static void on_mqtt_svc_event(mqtt_service_t *svc,
                              const mqtt_drv_event_t *event, void *user_data) {
  if (event->type == MQTT_DRV_EVENT_CONNECTED) {
    log_i("MQTT Connected, subscribing to command topics...");
    // 订阅所有设备的命令控制主题
    uint8_t count = thing_model_get_count();
    for (uint8_t i = 0; i < count; i++) {
      thing_device_t *dev = thing_model_get_device(i);
      if (dev && svc->adapter) {
        char topic[128];
        svc->adapter->get_topic(dev->device_id, MQTT_TOPIC_PROPERTY_SET, topic,
                                sizeof(topic));
        mqtt_svc_subscribe(svc, topic, 0);
        log_i("Subscribed to %s", topic);
      }
    }
  } else if (event->type == MQTT_DRV_EVENT_DATA) {
    if (svc->adapter) {
      char dev_id[64] = {0};
      char msg_id[64] = {0};
      if (svc->adapter->parse_command(event->topic, event->payload, dev_id,
                                      msg_id, on_prop_parsed, dev_id) == 0) {
        // 回复云端的命令
        if (strlen(msg_id) > 0) {
          mqtt_svc_reply_command(svc, dev_id, msg_id, 200); // 代码200表示成功
        }
      }
    }
  }
}

void cloud_bridge_init(mqtt_service_t *mqtt_svc) {
  g_mqtt_svc = mqtt_svc;

  // 监听MQTT服务事件
  mqtt_svc_register_callback(g_mqtt_svc, on_mqtt_svc_event, NULL);

  log_i("Cloud Bridge initialized.");
}

/**
 * @brief 云桥接处理函数
 *        周期同步设备属性到云端
 */
void cloud_bridge_process(void) {
  if (!g_mqtt_svc ||
      mqtt_svc_get_state(g_mqtt_svc) != MQTT_SVC_STATE_CONNECTED) {
    return; // 服务器未连接
  }

  // 检查同步周期
  uint32_t now = sys_get_systick_ms();
  if (now - g_last_sync_time < CLOUD_SYNC_INTERVAL_MS) {
    return;
  }
  g_last_sync_time = now;

  // log_v("prop sync...");
  //  扫描脏属性并同步
  uint8_t dev_count = thing_model_get_count();
  // 遍历所有设备
  for (uint8_t i = 0; i < dev_count; i++) {
    thing_device_t *dev = thing_model_get_device(i);
    if (!dev)
      continue;
    // 遍历此设备的所有属性
    for (uint8_t j = 0; j < dev->prop_count; j++) {
      thing_property_t *prop = &dev->properties[j];
      if (prop->is_dirty && prop->cloud_sync) { // 是否脏且需要同步
        log_d("Syncing dirty property %s.%s", dev->device_id, prop->id);
        if (mqtt_svc_publish_property(g_mqtt_svc, dev, prop) == 0) {
          prop->is_dirty = false; // 成功同步后，将属性标记为干净
        }
      }
    }
  }
}
