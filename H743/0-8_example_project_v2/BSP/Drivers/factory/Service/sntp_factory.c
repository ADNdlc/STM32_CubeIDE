#include "sntp_factory.h"
#include "at_controller.h"
#include <stddef.h>

#include "esp8266_sntp_driver.h"

static esp8266_sntp_driver_t esp_sntp_drv;
static bool initialized = false;

sntp_driver_t *sntp_driver_get(sntp_id_t id) {
  if (id >= SNTP_MAX_DEVICES)
    return NULL;

  if (!initialized) {
    extern at_controller_t *service_factory_get_at_controller(void);
    at_controller_t *sntp_at_ctrl = service_factory_get_at_controller();
    if (sntp_at_ctrl) {
      esp8266_sntp_driver_init(&esp_sntp_drv, sntp_at_ctrl);
      initialized = true;
    }
  }

  return (sntp_driver_t *)&esp_sntp_drv;
}
