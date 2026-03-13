/*
 * esp8266_sntp_driver.h
 *
 *  Created on: Jan 12, 2026
 *      Author: 12114
 */

#ifndef DEVICE_ESP_8266_ESP8266_SNTP_DRIVER_H_
#define DEVICE_ESP_8266_ESP8266_SNTP_DRIVER_H_

#include "at_controller.h"
#include "sntp_driver.h"

typedef struct {
  sntp_driver_t base;
  at_controller_t *at;
  sntp_drv_status_t status;

  // 异步查询回调上下文
  sntp_get_time_cb_t pending_cb;
  void *pending_user_data;
  char time_buf[64];
} esp8266_sntp_driver_t;

void esp8266_sntp_driver_init(esp8266_sntp_driver_t *self, at_controller_t *at);

#endif /* DEVICE_ESP_8266_ESP8266_SNTP_DRIVER_H_ */
