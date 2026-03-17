#include "mqtt_factory.h"
#include <stddef.h>

#include "esp_8266/service/esp8266_mqtt_driver.h"

static esp8266_mqtt_driver_t esp_mqtt_drv;
static bool initialized = false;

mqtt_driver_t *mqtt_driver_get(mqtt_id_t id) {
  if (id >= MQTT_MAX_DEVICES)
    return NULL;

  if (!initialized) {
    extern at_controller_t *service_factory_get_at_controller(void);
    at_controller_t *mqtt_at_ctrl = service_factory_get_at_controller();
    if (mqtt_at_ctrl) {
      esp8266_mqtt_driver_init(&esp_mqtt_drv, mqtt_at_ctrl);
      initialized = true;
    }
  }

  return (mqtt_driver_t *)&esp_mqtt_drv;
}
