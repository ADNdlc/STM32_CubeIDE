#ifndef COMPONENT_MQTT_SERVICE_MQTT_SERVICE_H_
#define COMPONENT_MQTT_SERVICE_MQTT_SERVICE_H_

#include "mqtt_adapter.h"
#include "mqtt_driver.h"
#include <stdbool.h>
#include <stdint.h>


/**
 * @brief 服务状态枚举
 */
typedef enum {
  MQTT_SVC_STATE_DISCONNECTED = 0,
  MQTT_SVC_STATE_CONNECTING,
  MQTT_SVC_STATE_CONNECTED,
  MQTT_SVC_STATE_FAULT
} mqtt_svc_state_t;

typedef struct mqtt_service_t mqtt_service_t;

// 状态变化回调原型
typedef void (*mqtt_svc_event_cb_t)(mqtt_service_t *self,
                                    mqtt_svc_state_t state, void *user_data);

/**
 * @brief mqtt服务结构体
 */
typedef struct mqtt_service_t {
  mqtt_driver_t *drv; // 驱动
  const mqtt_adapter_t *adapter; // 适配器(云平台)
  mqtt_svc_state_t state;        // 当前状态

  mqtt_svc_event_cb_t event_cb; // 状态变化回调
  void *user_data;              // 用户数据指针

  // 内部状态和计数器
  uint32_t last_reconnect_attempt;
  uint8_t retry_count;
} mqtt_service_t;

/**
 * @brief Initialize MQTT Service
 */
void mqtt_svc_init(mqtt_service_t *self, mqtt_driver_t *drv,
                   const mqtt_adapter_t *adapter);

/**
 * @brief Register a status changed callback
 */
void mqtt_svc_register_callback(mqtt_service_t *self, mqtt_svc_event_cb_t cb,
                                void *user_data);

/**
 * @brief Start the MQTT service (connect to broker)
 */
int mqtt_svc_connect(mqtt_service_t *self);

/**
 * @brief Disconnect from MQTT broker
 */
int mqtt_svc_disconnect(mqtt_service_t *self);

/**
 * @brief Process loop for MQTT service (reconnection, keep-alive if needed)
 */
void mqtt_svc_process(mqtt_service_t *self);

/**
 * @brief Publish a property update to the cloud
 */
int mqtt_svc_publish_property(mqtt_service_t *self,
                              const thing_device_t *device,
                              const thing_property_t *prop);

/**
 * @brief Get current service state
 */
mqtt_svc_state_t mqtt_svc_get_state(mqtt_service_t *self);

#endif /* COMPONENT_MQTT_SERVICE_MQTT_SERVICE_H_ */
