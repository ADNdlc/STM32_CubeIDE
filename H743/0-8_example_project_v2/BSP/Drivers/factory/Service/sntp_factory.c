#include "sntp_factory.h"
#include "esp8266_sntp_driver.h"
#include "at_controller.h"
#include <stddef.h>

extern at_controller_t* wifi_factory_get_at_controller(void);

static esp8266_sntp_driver_t esp_sntp_drv;
static bool initialized = false;

sntp_driver_t *sntp_driver_get(sntp_id_t id) {
    if (id >= SNTP_MAX_DEVICES) return NULL;

    if (!initialized) {
        at_controller_t *at = wifi_factory_get_at_controller();
        if (at) {
            esp8266_sntp_driver_init(&esp_sntp_drv, at);
            initialized = true;
        }
    }

    return (sntp_driver_t *)&esp_sntp_drv;
}
