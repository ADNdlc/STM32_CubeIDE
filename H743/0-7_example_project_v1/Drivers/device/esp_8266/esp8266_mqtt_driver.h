#ifndef DRIVERS_DEVICE_ESP_8266_ESP8266_MQTT_DRIVER_H_
#define DRIVERS_DEVICE_ESP_8266_ESP8266_MQTT_DRIVER_H_

#include "at_controller.h"
#include "mqtt_driver.h"

typedef struct {
  mqtt_driver_t base;           // 基础接口
  at_controller_t *at_ctrl;     // AT控制器
  mqtt_drv_event_cb_t event_cb; // 事件回调
  void *event_arg;              // 事件回调参数

  // Internal state
  bool is_connected; // 连接状态
} esp8266_mqtt_driver_t;

/**
 * @brief Initialize the ESP8266 MQTT Driver
 */
void esp8266_mqtt_driver_init(esp8266_mqtt_driver_t *self,
                              at_controller_t *at_ctrl);

#endif /* DRIVERS_DEVICE_ESP_8266_ESP8266_MQTT_DRIVER_H_ */
