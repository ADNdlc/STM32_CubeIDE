#include "cloud_bridge.h"
#include "sys.h"
#include <string.h>


#define LOG_TAG "CLOUD_BR"
#include "elog.h"

static mqtt_service_t *g_mqtt_svc = NULL;

// --- Helper: Find device by ID (Thing Model doesn't expose list easily yet)
// --- Actually we can use thing_model_get_device

// --- Thing Model Callback: Local change -> Publish to Cloud ---
static void on_thing_model_event(const thing_model_event_t *event,
                                 void *user_data) {
  if (event->type == THING_EVENT_PROPERTY_CHANGED) {
    // Note: Immediate publish removed in favor of periodic polling via is_dirty
    // flag.
    // This allows batching and prevents flooding during rapid changes.
  }
}

// --- MQTT Service Callback: Cloud Command -> Update Thing Model ---
static void on_mqtt_svc_event(mqtt_service_t *svc, mqtt_svc_state_t state,
                              void *user_data) {
  if (state == MQTT_SVC_STATE_CONNECTED) {
    log_i("CloudBridge: MQTT Connected, can subscribe/sync if needed.");
    // Note: Actual data handling is currently in mqtt_service.c
    // which calls thing_model_set_prop.
    // We might want to move that logic here for better decoupling of the
    // library.
  }
}

void cloud_bridge_init(mqtt_service_t *mqtt_svc) {
  g_mqtt_svc = mqtt_svc;

  // 1. 监听物模型的变化
  thing_model_add_observer(on_thing_model_event, NULL);

  // 2. 监听MQTT服务状态
  mqtt_svc_register_callback(g_mqtt_svc, on_mqtt_svc_event, NULL);

  log_i("Cloud Bridge initialized.");
}

#define CLOUD_SYNC_INTERVAL_MS 1000
static uint32_t g_last_sync_time = 0;

/**
 * @brief 云桥处理函数
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

  // 扫描脏属性并同步
  uint8_t dev_count = thing_model_get_count();
  for (uint8_t i = 0; i < dev_count; i++) { // 遍历所有设备
    thing_device_t *dev = thing_model_get_device(i);
    if (!dev)
      continue;

    for (uint8_t j = 0; j < dev->prop_count; j++) { // 遍历此设备的所有属性
      thing_property_t *prop = &dev->properties[j];
      if (prop->is_dirty && prop->cloud_sync) {     // 是否脏且需要同步
        log_d("CloudBridge: Syncing dirty property %s.%s", dev->device_id, prop->id);
        if (mqtt_svc_publish_property(g_mqtt_svc, dev, prop) == 0) {
          prop->is_dirty = false; // 成功同步后，将属性标记为干净
        }
      }
    }
  }
}
