#include "device_init.h"
#include "../../../Drivers/old_driver/led/led.h"
#include "thing_model.h"
#include <string.h>


#define LOG_TAG "DEV_INIT"
#include "../../../lib/EasyLogger/easylogger/inc/elog.h"

// --- Hardware Callback for LEDs ---
static bool led_prop_set_cb(struct thing_device_t *dev, const char *prop_id,
                            thing_value_t value) {
  if (strcmp(dev->device_id, "Light1") == 0) {
    if (strcmp(prop_id, "led0") == 0) {
      LED_SET(value.b, 0);
      log_i("Hardware: Light1.led0 set to %d", value.b);
      return true;
    }
  } else if (strcmp(dev->device_id, "Light2") == 0) {
    if (strcmp(prop_id, "led1") == 0) {
      LED_SET(value.b, 1);
      log_i("Hardware: Light2.led1 set to %d", value.b);
      return true;
    }
  }
  return false;
}

void sys_devices_init(void) {
  log_i("Initializing hardware devices for Thing Model...");

  // 1. Define Properties for Light1
  static thing_property_t light1_props[] = {{.id = "led0",
                                             .name = "Switch",
                                             .type = THING_PROP_TYPE_SWITCH,
                                             .access_mode = THING_ACCESS_RW}};

  thing_device_t light1_tmpl = {.device_id = "Light1",
                                .name = "Kitchen Light",
                                .prop_count = 1,
                                .properties = light1_props,
                                .on_prop_set = led_prop_set_cb};

  // 2. Define Properties for Light2
  static thing_property_t light2_props[] = {{.id = "led1",
                                             .name = "Switch",
                                             .type = THING_PROP_TYPE_SWITCH,
                                             .access_mode = THING_ACCESS_RW}};

  thing_device_t light2_tmpl = {.device_id = "Light2",
                                .name = "Bedroom Light",
                                .prop_count = 1,
                                .properties = light2_props,
                                .on_prop_set = led_prop_set_cb};

  // 3. Register with Thing Model
  thing_model_register(&light1_tmpl);
  thing_model_register(&light2_tmpl);

  log_i("Device registration completed.");
}
