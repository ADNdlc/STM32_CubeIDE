#include "cloud_bridge.h"
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
    // Only publish if the change didn't come from the cloud itself
    if (event->source != 1) {
      log_i("CloudBridge: Property %s.%s changed locally, publishing...",
            event->device_id, event->prop_id);

      // Find the device and property for publishing
      thing_device_t *dev = NULL;
      for (int i = 0; i < thing_model_get_count(); i++) {
        thing_device_t *d = thing_model_get_device(i);
        if (d && strcmp(d->device_id, event->device_id) == 0) {
          dev = d;
          break;
        }
      }

      if (dev) {
        // Find prop
        for (int j = 0; j < dev->prop_count; j++) {
          if (strcmp(dev->properties[j].id, event->prop_id) == 0) {
            mqtt_svc_publish_property(g_mqtt_svc, dev, &dev->properties[j]);
            break;
          }
        }
      }
    }
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

  // 1. Listen to Thing Model changes
  thing_model_add_observer(on_thing_model_event, NULL);

  // 2. Listen to MQTT service status
  mqtt_svc_register_callback(g_mqtt_svc, on_mqtt_svc_event, NULL);

  log_i("Cloud Bridge initialized.");
}
