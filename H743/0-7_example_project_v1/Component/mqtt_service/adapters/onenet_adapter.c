#include "home/System/sys_config.h"
#include "mqtt_adapter.h"
#include "util/cJSON.h"
#include <stdio.h>
#include <string.h>


/**
 * @brief OneNet specific implementation of the MQTT Platform Adapter
 */

static void onenet_get_conn_params(mqtt_conn_params_t *out_params) {
  const sys_config_t *cfg = sys_config_get();

  strcpy(out_params->host, "mqttst.heclouds.com"); // OneNet MQTT host
  out_params->port = 1883;

  // For OneNet, client_id is often the device_id
  strcpy(out_params->client_id, cfg->cloud.device_id);
  // username is the product_id
  strcpy(out_params->username, cfg->cloud.product_id);

  // password for OneNet often requires a token.
  // For now we use the device_secret as a fixed password if configured for
  // password login or return the secret directly as a placeholder.
  strcpy(out_params->password, cfg->cloud.device_secret);
}

static void onenet_get_post_topic(const char *device_id, char *out_topic,
                                  size_t size) {
  const sys_config_t *cfg = sys_config_get();
  snprintf(out_topic, size, "$sys/%s/%s/thing/property/post",
           cfg->cloud.product_id, device_id);
}

static void onenet_get_cmd_topic(const char *device_id, char *out_topic,
                                 size_t size) {
  const sys_config_t *cfg = sys_config_get();
  snprintf(out_topic, size, "$sys/%s/%s/thing/property/set",
           cfg->cloud.product_id, device_id);
}

static int onenet_serialize_post(const thing_device_t *device,
                                 const thing_property_t *prop, char *out_buf,
                                 size_t size) {
  cJSON *root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "id", "123");
  cJSON_AddStringToObject(root, "version", "1.0");

  cJSON *params = cJSON_CreateObject();
  cJSON *p_val = cJSON_CreateObject();

  switch (prop->type) {
  case THING_PROP_TYPE_SWITCH:
    cJSON_AddBoolToObject(p_val, "value", prop->value.b);
    break;
  case THING_PROP_TYPE_INT:
    cJSON_AddNumberToObject(p_val, "value", prop->value.i);
    break;
  case THING_PROP_TYPE_FLOAT:
    cJSON_AddNumberToObject(p_val, "value", prop->value.f);
    break;
  default:
    break;
  }

  cJSON_AddItemToObject(params, prop->id, p_val);
  cJSON_AddItemToObject(root, "params", params);

  char *json_str = cJSON_PrintUnformatted(root);
  if (json_str) {
    strncpy(out_buf, json_str, size - 1);
    cJSON_free(json_str);
  }
  cJSON_Delete(root);
  return 0;
}

static int onenet_parse_command(const char *payload, char *out_prop_id,
                                thing_value_t *out_value, char *out_msg_id) {
  cJSON *root = cJSON_Parse(payload);
  if (!root)
    return -1;

  cJSON *id = cJSON_GetObjectItem(root, "id");
  if (id)
    strcpy(out_msg_id, id->valuestring);

  cJSON *params = cJSON_GetObjectItem(root, "params");
  if (params && params->child) {
    strcpy(out_prop_id, params->child->string);
    // Simplified value parsing
    if (cJSON_IsBool(params->child)) {
      out_value->b = cJSON_IsTrue(params->child);
    } else if (cJSON_IsNumber(params->child)) {
      out_value->i = params->child->valueint; // or valuef
    }
  }

  cJSON_Delete(root);
  return 0;
}

static void onenet_get_reply_payload(const char *msg_id, int code,
                                     char *out_buf, size_t size) {
  snprintf(out_buf, size, "{\"id\":\"%s\",\"code\":%d,\"msg\":\"success\"}",
           msg_id, code);
}

static void onenet_get_reply_topic(const char *device_id, char *out_topic,
                                   size_t size) {
  const sys_config_t *cfg = sys_config_get();
  snprintf(out_topic, size, "$sys/%s/%s/thing/property/set_reply",
           cfg->cloud.product_id, device_id);
}

const mqtt_adapter_t g_onenet_adapter = {
    .get_conn_params = onenet_get_conn_params,
    .get_post_topic = onenet_get_post_topic,
    .get_cmd_topic = onenet_get_cmd_topic,
    .serialize_post = onenet_serialize_post,
    .parse_command = onenet_parse_command,
    .get_reply_payload = onenet_get_reply_payload,
    .get_reply_topic = onenet_get_reply_topic};
