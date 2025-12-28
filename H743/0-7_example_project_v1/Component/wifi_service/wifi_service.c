/*
 * wifi_service.c
 *
 *  Implementation of the high-level WiFi service.
 */

#include "wifi_service.h"
#include <string.h>

void wifi_service_init(wifi_service_t *self, wifi_driver_t *driver) {
  if (!self || !driver)
    return;
  memset(self, 0, sizeof(wifi_service_t));
  self->driver = driver;
}

void wifi_service_register_callback(wifi_service_t *self,
                                    wifi_service_event_cb_t cb,
                                    void *user_data) {
  if (!self)
    return;
  self->event_cb = cb;
  self->user_data = user_data;
}

void wifi_svc_process(wifi_service_t *self) {
  if (!self || !self->driver)
    return;

  // Poll driver status
  wifi_status_t current_status = WIFI_GET_STATUS(self->driver);

  // If status changed, notify via callback
  if (current_status != self->last_status) {
    if (self->event_cb) {
      self->event_cb(self, current_status, self->user_data);
    }
    self->last_status = current_status;
  }
}

int wifi_svc_connect(wifi_service_t *self, const char *ssid, const char *pwd) {
  if (!self || !self->driver)
    return -1;
  return WIFI_CONNECT(self->driver, ssid, pwd);
}

int wifi_svc_disconnect(wifi_service_t *self) {
  if (!self || !self->driver)
    return -1;
  return WIFI_DISCONNECT(self->driver);
}

int wifi_svc_scan(wifi_service_t *self, wifi_scan_cb_t cb, void *arg) {
  if (!self || !self->driver)
    return -1;
  return WIFI_SCAN(self->driver, cb, arg);
}

wifi_status_t wifi_svc_get_status(wifi_service_t *self) {
  if (!self || !self->driver)
    return WIFI_STATUS_DISCONNECTED;
  return WIFI_GET_STATUS(self->driver);
}

int wifi_svc_set_mode(wifi_service_t *self, wifi_mode_t mode) {
  if (!self || !self->driver)
    return -1;
  return WIFI_SET_MODE(self->driver, mode);
}

#include "../../Drivers/device/esp_8266/esp8266_wifi_driver.h"

uint16_t wifi_svc_get_scan_count(wifi_service_t *self) {
  if (!self || !self->driver)
    return 0;
  // TODO: Add generic way to get count if needed. For now cast to ESP8266.
  esp8266_wifi_driver_t *drv = (esp8266_wifi_driver_t *)self->driver;
  return drv->scan_count;
}

wifi_ap_info_t *wifi_svc_get_scan_results(wifi_service_t *self) {
  if (!self || !self->driver)
    return NULL;
  esp8266_wifi_driver_t *drv = (esp8266_wifi_driver_t *)self->driver;
  return drv->scan_results;
}
