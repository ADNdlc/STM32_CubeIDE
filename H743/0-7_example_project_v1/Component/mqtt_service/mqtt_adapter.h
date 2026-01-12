#ifndef COMPONENT_MQTT_SERVICE_MQTT_ADAPTER_H_
#define COMPONENT_MQTT_SERVICE_MQTT_ADAPTER_H_

#include "thing_model/thing_model.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief MQTT 登录信息
 */
typedef struct {
  char host[64];       // 服务器地址
  uint16_t port;       // 服务器端口
  char client_id[128]; // 客户端ID
  char username[128];  // 用户名
  char password[256];  // 密码
} mqtt_conn_params_t;

/**
 * @brief  属性解析回调(parse_command每解析出一个属性就调用一次)
 *  @param prop_id 属性ID
 *  @param value 属性值
 *  @param ctx 回调上下文
 */
typedef void (*thing_on_prop_parsed_cb)(const char *prop_id,
                                        thing_value_t value, void *ctx);

/**
 * @brief 平台适配接口
 */
typedef struct {
  /**
   * @brief生成连接参数
   */
  void (*get_conn_params)(mqtt_conn_params_t *out_params);

  /**
   * @brief 生成发布主题
   * @param device_id 设备ID
   * @param out_topic 输出主题
   * @param size 输出主题缓冲区大小
   */
  void (*get_post_topic)(const char *device_id, char *out_topic, size_t size);

  /**
   * @brief 生成订阅主题
   * @param device_id 设备ID
   * @param out_topic 输出主题
   * @param size 输出主题缓冲区大小
   */
  void (*get_cmd_topic)(const char *device_id, char *out_topic, size_t size);

  /**
   * @brief 序列化物模型属性
   * @param device 设备信息
   * @param prop 属性信息
   * @param out_buf 输出缓冲区
   * @param size 输出缓冲区大小
   */
  int (*serialize_post)(const thing_device_t *device,
                        const thing_property_t *prop, char *out_buf,
                        size_t size);

  /**
   * @brief 解析云平台命令
   * @param topic 主题
   * @param payload 数据
   * @param out_device_id 输出设备ID
   * @param out_msg_id 输出消息ID
   * @param prop_cb 属性解析回调 (每解析到一个属性调用一次)
   * @param ctx 回调上下文
   */
  int (*parse_command)(const char *topic, const char *payload,
                       char *out_device_id, char *out_msg_id,
                       thing_on_prop_parsed_cb prop_cb, void *ctx);

  /**
   * @brief 生成命令回复数据
   * @param msg_id 消息ID
   * @param code 状态码
   * @param out_buf 输出缓冲区
   * @param size 输出缓冲区大小
   */
  void (*get_reply_payload)(const char *msg_id, int code, char *out_buf,
                            size_t size);

  /**
   * @brief 生成回复主题
   * @param device_id 设备ID
   * @param out_topic 输出主题
   * @param size 输出主题缓冲区大小
   */
  void (*get_reply_topic)(const char *device_id, char *out_topic, size_t size);

} mqtt_adapter_t;

#endif /* COMPONENT_MQTT_SERVICE_MQTT_ADAPTER_H_ */
