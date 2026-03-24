#include "wifi_factory.h"
#include <stddef.h>

#include "esp_8266/service/esp8266_wifi_driver.h"

static esp8266_wifi_driver_t esp_wifi_drv;
static bool initialized = false;

wifi_driver_t *wifi_driver_get(wifi_id_t id) {
  if (id >= WIFI_MAX_DEVICES)
    return NULL;

  if (!initialized) {
    extern at_controller_t *service_factory_get_at_controller(void);
    // 获取底层驱动
    at_controller_t *wifi_at_ctrl = service_factory_get_at_controller();
    if (wifi_at_ctrl) {
      // 初始化 WiFi 驱动
      esp8266_wifi_driver_init(&esp_wifi_drv, wifi_at_ctrl);
    }
  }

  return (wifi_driver_t *)&esp_wifi_drv;
}
