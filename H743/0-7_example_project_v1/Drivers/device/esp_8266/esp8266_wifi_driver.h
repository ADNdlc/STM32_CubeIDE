/*
 * esp8266_wifi_driver.h
 *
 *  ESP8266 implementation of the wifi_driver interface.
 */

#ifndef DRIVERS_DEVICE_ESP_8266_ESP8266_WIFI_DRIVER_H_
#define DRIVERS_DEVICE_ESP_8266_ESP8266_WIFI_DRIVER_H_

#include "at_controller.h"
#include "wifi_driver.h"

typedef struct {
  wifi_driver_t base;       // 驱动接口
  at_controller_t *at_ctrl; // AT控制器
  wifi_status_t status;     // 当前状态
  wifi_mode_t mode;         // 当前模式

  // AP扫描结果
  wifi_ap_info_t scan_results[10];
  uint16_t scan_count;

  // 扫描回调
  wifi_scan_cb_t scan_cb;
  void *scan_arg;
} esp8266_wifi_driver_t;

/**
 * @brief Initialize the ESP8266 WiFi driver
 */
void esp8266_wifi_driver_init(esp8266_wifi_driver_t *self,
                              at_controller_t *at_ctrl);

#endif /* DRIVERS_DEVICE_ESP_8266_ESP8266_WIFI_DRIVER_H_ */
