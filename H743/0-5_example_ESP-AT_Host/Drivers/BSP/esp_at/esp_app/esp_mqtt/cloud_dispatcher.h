/*
 * cloud_dispatcher.h
 *
 *  Created on: Aug 21, 2025
 *      Author: 12114
 */

#ifndef BSP_ESP_AT_ESP_APP_ESP_MQTT_CLOUD_DISPATCHER_H_
#define BSP_ESP_AT_ESP_APP_ESP_MQTT_CLOUD_DISPATCHER_H_

#include "stdbool.h"
#include "../cJSON.h"

//#define oldmethod



// 定义云命令处理函数的原型
// 参数是 cJSON 对象，这样可以灵活处理各种数据类型 (bool, number, string)
typedef void (*cloud_cmd_handler_t)(cJSON* value_item);

#ifdef oldmethod
/**
 * @brief 向云命令注册表注册一个新的命令处理器
 * @param identifier 命令的标识符 (JSON key), e.g., "LED"
 * @param handler    对应的处理函数
 * @return true 注册成功, false 注册失败 (例如，表满了)
 */
bool Cloud_dispatcher_register_handler(const char* identifier, cloud_cmd_handler_t handler);

#endif

/**
 * @brief 解析并分发来自云端的命令
 *        这个函数由 esp_mqtt.c 中的 MQTT_handle_urc_recv 调用
 * @param payload_json 包含命令的JSON字符串
 */
void Cloud_dispatcher_process_command(const char* payload_json);

#endif /* BSP_ESP_AT_ESP_APP_ESP_MQTT_CLOUD_DISPATCHER_H_ */
