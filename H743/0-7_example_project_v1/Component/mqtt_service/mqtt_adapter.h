#ifndef COMPONENT_MQTT_SERVICE_MQTT_ADAPTER_H_
#define COMPONENT_MQTT_SERVICE_MQTT_ADAPTER_H_

#include "home/System/thing_model.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief MQTT Credentials for login
 */
typedef struct {
  char host[64];
  uint16_t port;
  char client_id[128];
  char username[128];
  char password[128];
} mqtt_conn_params_t;

/**
 * @brief MQTT Platform Adapter Interface
 */
typedef struct {
  /**
   * @brief Generate connection parameters (host, port, user, pwd)
   *        based on device configuration.
   */
  void (*get_conn_params)(mqtt_conn_params_t *out_params);

  /**
   * @brief Generate the topic for property publishing
   */
  void (*get_post_topic)(const char *device_id, char *out_topic, size_t size);

  /**
   * @brief Generate the topic for command receiving (subscription)
   */
  void (*get_cmd_topic)(const char *device_id, char *out_topic, size_t size);

  /**
   * @brief Pack a property update into the platform's JSON format
   */
  int (*serialize_post)(const thing_device_t *device,
                        const thing_property_t *prop, char *out_buf,
                        size_t size);

  /**
   * @brief Parse an incoming command from the platform
   */
  int (*parse_command)(const char *topic, const char *payload,
                       char *out_device_id, char *out_prop_id,
                       thing_value_t *out_value, char *out_msg_id);

  /**
   * @brief Generate a reply message for a received command
   */
  void (*get_reply_payload)(const char *msg_id, int code, char *out_buf,
                            size_t size);

  /**
   * @brief Generate the topic for the reply
   */
  void (*get_reply_topic)(const char *device_id, char *out_topic, size_t size);

} mqtt_adapter_t;

#endif /* COMPONENT_MQTT_SERVICE_MQTT_ADAPTER_H_ */
