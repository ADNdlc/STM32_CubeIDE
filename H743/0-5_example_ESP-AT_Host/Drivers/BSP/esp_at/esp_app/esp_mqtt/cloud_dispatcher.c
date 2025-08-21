/*
 * cloud_dispatcher.c
 *
 *  Created on: Aug 21, 2025
 *      Author: 12114
 */

#include "cloud_dispatcher.h"
#include "esp_mqtt.h"
#include <string.h>
#include <stdio.h>

#define MAX_CLOUD_HANDLERS 16 // 定义最大可注册的命令数量(可写功能点数量)

// 命令注册表条目
typedef struct {
    const char* identifier;
    cloud_cmd_handler_t handler;
} Cloud_Handler_t;

// 命令注册表实例
static Cloud_Handler_t handler_registry[MAX_CLOUD_HANDLERS];
static int handler_count = 0;	//已有条目数

bool Cloud_dispatcher_register_handler(const char* identifier, cloud_cmd_handler_t handler) {
    if (handler_count >= MAX_CLOUD_HANDLERS) {
        return false; // 表已满
    }
    handler_registry[handler_count].identifier = identifier;
    handler_registry[handler_count].handler = handler;
    handler_count++;
    return true;
}

void Cloud_dispatcher_process_command(const char* payload_json) {
    cJSON* root = cJSON_Parse(payload_json);
    if (root == NULL) {
        printf("Error: Failed to parse JSON payload.\r\n");
        return;
    }
    // 提取重要的元数据 (id)
    cJSON* id_item = cJSON_GetObjectItem(root, "id");
    if (id_item == NULL || !cJSON_IsString(id_item)) {
        cJSON_Delete(root);
        return;
    }
    const char* cmd_id = id_item->valuestring;

    // 获取 params 对象
    cJSON* params = cJSON_GetObjectItem(root, "params");
    if (params == NULL || !cJSON_IsObject(params)) {
        cJSON_Delete(root);
        // 即使没有params，也应该回复
        MQTT_send_reply(cmd_id, 200, "success"); // 假设有个这个函数
        return;
    }

    // 遍历 params 对象中的所有条目(功能点标识符)
    cJSON* current_param = params->child;
    while (current_param != NULL) {
        const char* identifier = current_param->string; // 这就是 "LED", "angle"

        // 在注册表中查找匹配的处理器
        bool handled = false;
        for (int i = 0; i < handler_count; ++i) {
            if (strcmp(handler_registry[i].identifier, identifier) == 0) {
                // 找到了！调用它
                handler_registry[i].handler(current_param);
                handled = true;
                break;
            }
        }
        if (!handled) {
            printf("Warning: No handler for identifier '%s'\r\n", identifier);
        }

        current_param = current_param->next; // 移动到下一个参数
    }

    cJSON_Delete(root);// 清理JSON对象

    // 立即构建并发送成功回复 (这个函数将在 esp_mqtt.c 中实现)
    extern void MQTT_send_reply(const char* id, int code, const char* msg);
    MQTT_send_reply(cmd_id, 200, "success");
}
