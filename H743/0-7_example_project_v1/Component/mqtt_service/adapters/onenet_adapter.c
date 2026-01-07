#include "cJSON.h"
#include "home/System/sys_config.h"
#include "mqtt_adapter.h"
#include <stdio.h>
#include <string.h>

#define ONENET_SERVER_HOST "mqtts.heclouds.com"
#define ONENET_SERVER_PORT 1883

/**
 * @brief 获取OneNet连接参数
 */

static void onenet_get_conn_params(mqtt_conn_params_t *out_params) {
  const sys_config_t *cfg = sys_config_get();

  strcpy(out_params->host, ONENET_SERVER_HOST);
  out_params->port = ONENET_SERVER_PORT;

  // 对于 OneNet, client_id是设备id
  strcpy(out_params->client_id, cfg->cloud.device_id);
  // username是产品id
  strcpy(out_params->username, cfg->cloud.product_id);
  // password
  strcpy(out_params->password, cfg->cloud.device_secret);
}

/**
 * @brief 获取OneNet物模型属性发布主题
 */
static void onenet_get_post_topic(const char *device_id, char *out_topic,
                                  size_t size) {
  const sys_config_t *cfg = sys_config_get();
  snprintf(out_topic, size, "$sys/%s/%s/thing/property/post",
           cfg->cloud.product_id, device_id);
}

/**
 * @brief 获取OneNet物模型属性订阅主题
 */
static void onenet_get_cmd_topic(const char *device_id, char *out_topic,
                                 size_t size) {
  const sys_config_t *cfg = sys_config_get();
  snprintf(out_topic, size, "$sys/%s/%s/thing/property/set",
           cfg->cloud.product_id, device_id);
}

/**
 * @brief 序列化OneNet物模型属性
 */
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

/**
 * @brief 解析OneNet物模型属性
 */
static int onenet_parse_command(const char *topic, const char *payload,
                                char *out_device_id, char *out_msg_id,
                                thing_on_prop_parsed_cb prop_cb, void *ctx) {
  // 1. Extract device ID from topic: $sys/{pid}/{did}/thing/property/set
  char topic_copy[128];
  strncpy(topic_copy, topic, sizeof(topic_copy) - 1);
  char *save_ptr;
  char *segment = strtok_r(topic_copy, "/", &save_ptr); // $sys
  segment = strtok_r(NULL, "/", &save_ptr);             // {pid}
  segment = strtok_r(NULL, "/", &save_ptr);             // {did}
  if (segment) {
    strcpy(out_device_id, segment);
  } else {
    return -1;
  }

  // 2. Parse JSON payload
  cJSON *root = cJSON_Parse(payload);
  if (!root)
    return -1;

  cJSON *id = cJSON_GetObjectItem(root, "id");
  if (id)
    strcpy(out_msg_id, id->valuestring);

  cJSON *params = cJSON_GetObjectItem(root, "params");
  if (params && prop_cb) {
    // Iterate through all children of params
    cJSON *prop = params->child;
    while (prop) {
      thing_value_t out_value;
      bool valid = false;

      if (cJSON_IsBool(prop)) {
        out_value.b = cJSON_IsTrue(prop);
        valid = true;
      } else if (cJSON_IsNumber(prop)) {
        out_value.i = prop->valueint; // or valuef
        valid = true;
      } else if (cJSON_IsString(prop)) {
        out_value.s = prop->valuestring;
        valid = true;
      }

      if (valid) {
        prop_cb(prop->string, out_value, ctx);
      }
      prop = prop->next;
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
