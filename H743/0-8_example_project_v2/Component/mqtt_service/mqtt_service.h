#ifndef COMPONENT_MQTT_SERVICE_MQTT_SERVICE_H_
#define COMPONENT_MQTT_SERVICE_MQTT_SERVICE_H_

#include "mqtt_adapter.h"
#include "mqtt_driver.h"	// 驱动接口
#include "service_id.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief 服务状态枚举
 */
typedef enum {
  MQTT_SVC_STATE_DISCONNECTED = 0, // 未连接
  MQTT_SVC_STATE_CONNECTING,       // 连接中
  MQTT_SVC_STATE_CONNECTED,        // 已连接
  MQTT_SVC_STATE_FAULT             // 错误
} mqtt_svc_state_t;


typedef struct mqtt_service_t mqtt_service_t;


/**
 * @brief 事件处理回调原型
 *
 * @param self 服务实例
 * @param event 事件类型
 * @param user_data 事件回调参数
 */
typedef void (*mqtt_svc_event_cb_t)(mqtt_service_t *self,
                                    const mqtt_drv_event_t *event,
                                    void *user_data);
/**
 * @brief MQTT 服务观察者结构体
 */
typedef struct {
  mqtt_svc_event_cb_t cb;
  void *user_data;
} mqtt_svc_observer_t;

#define MAX_MQTT_SVC_OBSERVERS 4	//定义最大观察者数量

/**
 * @brief mqtt服务结构体
 */
typedef struct mqtt_service_t {
  mqtt_driver_t *drv;            // 驱动
  const mqtt_adapter_t *adapter; // 适配器(云平台)
  mqtt_svc_state_t state;        // 当前状态

  mqtt_svc_observer_t observers[MAX_MQTT_SVC_OBSERVERS]; // 观察者列表
  uint8_t observer_count;	// 已注册的数量

  // 内部状态和计数器
  uint32_t last_reconnect_attempt; // 最后连接时间
  uint8_t retry_count;			   // 重试次数
} mqtt_service_t;

/**
 * @brief Initialize MQTT Service
 */
void mqtt_svc_init(mqtt_service_t *self, mqtt_driver_t *drv, const mqtt_adapter_t *adapter);

/**
 * @brief 注册一个事件回调 (支持多个观察者)
 */
int mqtt_svc_register_callback(mqtt_service_t *self, mqtt_svc_event_cb_t cb, void *user_data);

/**
 * @brief 连接mqtt服务器
 */
int mqtt_svc_connect(mqtt_service_t *self);

/**
 * @brief 断开mqtt连接
 */
int mqtt_svc_disconnect(mqtt_service_t *self);

/**
 * @brief 订阅主题
 */
int mqtt_svc_subscribe(mqtt_service_t *self, const char *topic, uint8_t qos);

/**
 * @brief 发布属性到云平台
 * @param self mqtt服务实例
 * @param device 设备信息
 * @param prop 属性信息
 * @return int 0:成功，-1:失败
 */
int mqtt_svc_publish_property(mqtt_service_t *self,
                              const thing_device_t *device,
                              const thing_property_t *prop);

/**
 * @brief 云命令回复
 */
void mqtt_svc_reply_command(mqtt_service_t *self, const char *device_id,
                            const char *msg_id, int code);

/**
 * @brief 获取当前服务状态
 */
mqtt_svc_state_t mqtt_svc_get_state(mqtt_service_t *self);

/**
 * @brief mqtt服务处理循环
 */
void mqtt_svc_process(mqtt_service_t *self);



#endif /* COMPONENT_MQTT_SERVICE_MQTT_SERVICE_H_ */
