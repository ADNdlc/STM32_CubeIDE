#ifndef DRIVERS_DEVICE_ESP_8266_ESP8266_MQTT_DRIVER_H_
#define DRIVERS_DEVICE_ESP_8266_ESP8266_MQTT_DRIVER_H_

#include "mqtt_driver.h"
#include "at_controller.h"

typedef struct {
  mqtt_driver_t base;
  at_controller_t *at_ctrl;
  mqtt_drv_event_cb_t event_cb;
  void *event_arg;

  // Internal state
  bool is_connected;
} esp8266_mqtt_driver_t;

/**
 * @brief Initialize the ESP8266 MQTT Driver
 */
void esp8266_mqtt_driver_init(esp8266_mqtt_driver_t *self,
                              at_controller_t *at_ctrl);

#endif /* DRIVERS_DEVICE_ESP_8266_ESP8266_MQTT_DRIVER_H_ */
