#include "cJSON.h"
#include "onenet_adapter.h"
#include <stdio.h>
#include <string.h>

#define ONENET_SERVER_HOST "mqtts.heclouds.com"
#define ONENET_SERVER_PORT 1883

static onenet_config_t s_onenet_cfg; // 运行时参数

/**
 * @brief 初始化 OneNet 适配器参数
 */
void onenet_adapter_init(const onenet_config_t *config) {
  if (config) {
    memcpy(&s_onenet_cfg, config, sizeof(onenet_config_t));
  }
}

/**
 * @brief 获取OneNet平台的MQTT连接参数
 */
static void onenet_get_conn_params(mqtt_conn_params_t *out_params) {
  strcpy(out_params->host, ONENET_SERVER_HOST);
  out_params->port = ONENET_SERVER_PORT;

  // 对于 OneNet, client_id是设备id
  strcpy(out_params->client_id, s_onenet_cfg.device_id);
  // username是产品id
  strcpy(out_params->username, s_onenet_cfg.product_id);
  // password
  strcpy(out_params->password, s_onenet_cfg.device_secret);
}

/**
 * @brief 获取OneNet物模型属性发布/订阅等主题
 */
static void onenet_get_topic(const char *device_id, mqtt_topic_type_t topic_type,
                             char *out_topic, size_t size) {
  switch (topic_type) {
    case MQTT_TOPIC_PROPERTY_POST:
      snprintf(out_topic, size, "$sys/%s/%s/thing/property/post",
               s_onenet_cfg.product_id, device_id);
      break;
    case MQTT_TOPIC_PROPERTY_SET:
      snprintf(out_topic, size, "$sys/%s/%s/thing/property/set",
               s_onenet_cfg.product_id, device_id);
      break;
    case MQTT_TOPIC_PROPERTY_SET_REPLY:
      snprintf(out_topic, size, "$sys/%s/%s/thing/property/set_reply",
               s_onenet_cfg.product_id, device_id);
      break;
    default:
      if (size > 0) out_topic[0] = '\0';
      break;
  }
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

  // TODO: 时间戳可以作为参数或由外部注入，这里简单传入 0 表示不需要，如有需要由应用处理
  // cJSON_AddNumberToObject(p_val, "time", (double)timestamp);

  cJSON_AddItemToObject(params, prop->id, p_val);
  cJSON_AddItemToObject(root, "params", params);

  char *json_str = cJSON_PrintUnformatted(root);
  if (json_str) {
    strncpy(out_buf, json_str, size - 1);
    out_buf[size - 1] = '\0'; // Ensure null termination
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
  topic_copy[sizeof(topic_copy) - 1] = '\0';
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
  if (id && cJSON_IsString(id))
    strcpy(out_msg_id, id->valuestring);

  cJSON *params = cJSON_GetObjectItem(root, "params");
  if (params && prop_cb) {
    // 遍历属性
    cJSON *prop = params->child;
    while (prop) {
      thing_value_t out_value;
      bool valid = false;
      // 检查属性类型
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

const mqtt_adapter_t g_onenet_adapter = {
    .get_conn_params = onenet_get_conn_params,
    .get_topic = onenet_get_topic,
    .serialize_post = onenet_serialize_post,
    .parse_command = onenet_parse_command,
    .get_reply_payload = onenet_get_reply_payload,
};
