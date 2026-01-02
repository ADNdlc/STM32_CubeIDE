#include "thing_model.h"
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "THING_MODEL"
#include "elog.h"

#define MAX_THING_DEVICES 16

static thing_device_t *g_devices[MAX_THING_DEVICES];
static uint8_t g_device_count = 0;

void thing_model_init(void) {
  memset(g_devices, 0, sizeof(g_devices));
  g_device_count = 0;
  log_i("Thing Model Manager initialized.");
}

thing_device_t *thing_model_register(const thing_device_t *tmpl) {
  if (g_device_count >= MAX_THING_DEVICES) {
    log_e("Maximum device count reached.");
    return NULL;
  }

  // Allocate memory for device structure
  thing_device_t *dev = (thing_device_t *)malloc(sizeof(thing_device_t));
  if (!dev)
    return NULL;

  memcpy(dev, tmpl, sizeof(thing_device_t));

  // Deep copy properties
  if (tmpl->prop_count > 0) {
    dev->properties =
        (thing_property_t *)malloc(sizeof(thing_property_t) * tmpl->prop_count);
    if (dev->properties) {
      memcpy(dev->properties, tmpl->properties,
             sizeof(thing_property_t) * tmpl->prop_count);
    } else {
      free(dev);
      return NULL;
    }
  }

  g_devices[g_device_count++] = dev;
  log_i("Registered device: %s (%s)", dev->name, dev->device_id);

  return dev;
}

bool thing_model_set_prop(const char *device_id, const char *prop_id,
                          thing_value_t value, int source) {
  // 1. Find the device
  thing_device_t *target_dev = NULL;
  for (int i = 0; i < g_device_count; i++) {
    if (strcmp(g_devices[i]->device_id, device_id) == 0) {
      target_dev = g_devices[i];
      break;
    }
  }

  if (!target_dev)
    return false;

  // 2. Find the property
  thing_property_t *target_prop = NULL;
  for (int j = 0; j < target_dev->prop_count; j++) {
    if (strcmp(target_dev->properties[j].id, prop_id) == 0) {
      target_prop = &target_dev->properties[j];
      break;
    }
  }

  if (!target_prop)
    return false;

  // 3. Trigger Hardware Callback (if applicable and not from driver itself)
  if (source != 2 && target_dev->on_prop_set) {
    if (!target_dev->on_prop_set(target_dev, prop_id, value)) {
      log_w("Driver rejected property update: %s.%s", device_id, prop_id);
      return false;
    }
  }

  // 4. Update internal value
  // Note: for STRING types, we might need a more complex copy if it's not a
  // constant
  target_prop->value = value;

  // 5. Notify Observers (Logic to be implemented: UI Refresh and Cloud Post)
  // For now, we just log it.
  log_i("[Event] Prop %s.%s updated to value by source %d", device_id, prop_id,
        source);

  return true;
}

thing_device_t *thing_model_get_device(uint8_t index) {
  if (index < g_device_count)
    return g_devices[index];
  return NULL;
}

uint8_t thing_model_get_count(void) { return g_device_count; }
