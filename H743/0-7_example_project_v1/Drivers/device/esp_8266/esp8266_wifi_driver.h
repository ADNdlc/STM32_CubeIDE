/*
 * esp8266_wifi_driver.h
 *
 *  ESP8266 implementation of the wifi_driver interface.
 */

#ifndef DRIVERS_DEVICE_ESP_8266_ESP8266_WIFI_DRIVER_H_
#define DRIVERS_DEVICE_ESP_8266_ESP8266_WIFI_DRIVER_H_

#include "../../interface/wifi_driver.h"
#include "at_controller.h"

typedef struct {
  wifi_driver_t base;       // Inherit from wifi_driver_t
  at_controller_t *at_ctrl; // Pointer to the AT controller
  wifi_status_t status;
  wifi_mode_t mode;

  // Scan Results
  wifi_ap_info_t scan_results[20];
  uint16_t scan_count;
} esp8266_wifi_driver_t;

/**
 * @brief Initialize the ESP8266 WiFi driver
 */
void esp8266_wifi_driver_init(esp8266_wifi_driver_t *self,
                              at_controller_t *at_ctrl);

#endif /* DRIVERS_DEVICE_ESP_8266_ESP8266_WIFI_DRIVER_H_ */
