#ifndef DRIVERS_INTERFACE_MQTT_DRIVER_H_
#define DRIVERS_INTERFACE_MQTT_DRIVER_H_

#include <stdbool.h>
#include <stdint.h>


/**
 * @brief MQTT Connection Parameters
 */
typedef struct {
  const char *host;
  uint16_t port;
  const char *client_id;
  const char *username;
  const char *password;
  uint16_t keepalive;
} mqtt_driver_conn_params_t;

/**
 * @brief MQTT Events
 */
typedef enum {
  MQTT_DRV_EVENT_CONNECTED,
  MQTT_DRV_EVENT_DISCONNECTED,
  MQTT_DRV_EVENT_DATA,
} mqtt_drv_event_type_t;

typedef struct {
  mqtt_drv_event_type_t type;
  const char *topic;
  const char *payload;
  uint16_t payload_len;
} mqtt_drv_event_t;

/**
 * @brief Event Callback
 */
typedef void (*mqtt_drv_event_cb_t)(void *arg, mqtt_drv_event_t *event);

typedef struct mqtt_driver_t mqtt_driver_t;

/**
 * @brief MQTT Driver Interface (VTable)
 */
typedef struct {
  int (*connect)(mqtt_driver_t *self, const mqtt_driver_conn_params_t *params);
  int (*disconnect)(mqtt_driver_t *self);
  int (*publish)(mqtt_driver_t *self, const char *topic, const char *payload,
                 int qos);
  int (*subscribe)(mqtt_driver_t *self, const char *topic, int qos);
  void (*set_event_callback)(mqtt_driver_t *self, mqtt_drv_event_cb_t cb,
                             void *arg);
} mqtt_driver_ops_t;

struct mqtt_driver_t {
  const mqtt_driver_ops_t *ops;
};

// Helper Macros
#define MQTT_DRV_CONNECT(d, p) (d)->ops->connect(d, p)
#define MQTT_DRV_DISCONNECT(d) (d)->ops->disconnect(d)
#define MQTT_DRV_PUBLISH(d, t, p, q) (d)->ops->publish(d, t, p, q)
#define MQTT_DRV_SUBSCRIBE(d, t, q) (d)->ops->subscribe(d, t, q)
#define MQTT_DRV_SET_CB(d, c, a) (d)->ops->set_event_callback(d, c, a)

#endif /* DRIVERS_INTERFACE_MQTT_DRIVER_H_ */
