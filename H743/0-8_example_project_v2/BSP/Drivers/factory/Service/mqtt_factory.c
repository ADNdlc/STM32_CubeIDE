#include "mqtt_factory.h"
#include "esp8266_mqtt_driver.h"
#include "at_controller.h"
#include <stddef.h>

// 外部引用或共享 AT 控制器
extern at_controller_t* wifi_factory_get_at_controller(void);

static esp8266_mqtt_driver_t esp_mqtt_drv;
static bool initialized = false;

mqtt_driver_t *mqtt_driver_get(mqtt_id_t id) {
    if (id >= MQTT_MAX_DEVICES) return NULL;

    if (!initialized) {
        at_controller_t *at = wifi_factory_get_at_controller();
        if (at) {
            esp8266_mqtt_driver_init(&esp_mqtt_drv, at);
            initialized = true;
        }
    }

    return (mqtt_driver_t *)&esp_mqtt_drv;
}
