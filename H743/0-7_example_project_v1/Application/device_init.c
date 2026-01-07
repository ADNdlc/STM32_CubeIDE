#include "device_init.h"
#include "gpio_led/gpio_led.h"
#include "thing_model.h"
#include "sys_config.h"
#include <string.h>

#define LOG_TAG "DEV_INIT"
#include "elog.h"

static cloud_config_t* config = NULL;

#if 1
static bool device_prop_set_cb(struct thing_device_t *dev, const char *prop_id, thing_value_t value)
{
  if (strcmp(dev->device_id, config->device_id) == 0)
  {
    if (strcmp(prop_id, "led0") == 0)
    {
      log_i("Hardware: led0 set to %d", value.b);
      return true;
    }
    else if (strcmp(prop_id, "led1") == 0)
    {
      log_i("Hardware: led1 set to %d", value.b);
      return true;
    }
  }
  return false;
}

void sys_devices_init(void)
{
  log_i("Initializing hardware devices for Thing Model...");
  sys_config_t *sys_config = sys_config_get();
  config = &(sys_config->cloud);

  static thing_property_t device_props[] = {
      {.id = "led0",
       .name = "Switch",
       .type = THING_PROP_TYPE_SWITCH,
       .cloud_sync = true},
      {.id = "led1",
       .name = "Switch",
       .type = THING_PROP_TYPE_SWITCH,
       .cloud_sync = true},
  };

  thing_device_t device_tmpl = {.device_id = config->device_id,
                                .name = "TestDev",
                                .prop_count = 2,
                                .properties = device_props,
                                .on_prop_set = device_prop_set_cb};

  // 3. 注册物模型
  thing_model_register(&device_tmpl);

  log_i("Device registration completed.");
}
#endif
