#ifndef DRIVERS_INTERFACE_MQTT_DRIVER_H_
#define DRIVERS_INTERFACE_MQTT_DRIVER_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief 连接参数
 */
typedef struct {
  const char *host;      // 服务器地址
  uint16_t port;         // 服务器端口
  const char *client_id; // 客户端ID
  const char *username;  // 用户名
  const char *password;  // 密码
  uint16_t keepalive;    // 保持连接时间间隔(秒)
} mqtt_driver_conn_params_t;

/**
 * @brief MQTT事件类型
 */
typedef enum {
  MQTT_DRV_EVENT_CONNECTED,     // 已连接
  MQTT_DRV_EVENT_DISCONNECTED,  // 已断开
  MQTT_DRV_EVENT_DATA,          // 收到订阅数据
} mqtt_drv_event_type_t;

/**
 * @brief MQTT事件体
 */
typedef struct {
  mqtt_drv_event_type_t type; // 事件类型
  const char *topic;          // 主题(仅DATA事件有效)
  const char *payload;        // 数据
  uint16_t payload_len;       // 数据长度
} mqtt_drv_event_t;

/**
 * @brief 事件处理回调原型
 *
 * @param arg 事件回调参数
 * @param event 事件类型
 */
typedef void (*mqtt_drv_event_cb_t)(void *arg, mqtt_drv_event_t *event);

typedef struct mqtt_driver_t mqtt_driver_t;

/**
 * @brief MQTT驱动接口
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
