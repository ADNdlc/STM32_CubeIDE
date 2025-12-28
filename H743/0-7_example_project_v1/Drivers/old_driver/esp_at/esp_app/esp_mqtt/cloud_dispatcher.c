/*
 * cloud_dispatcher.c
 *
 *  Created on: Aug 21, 2025
 *      Author: 12114
 */

#include "cloud_dispatcher.h"
//#include "service/device_manager.h"
#include "esp_mqtt.h"
#include <string.h>
#include <stdio.h>
#include "cJSON.h"

#if useoldfunc
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
#else
static bool convert_cjson_to_prop_value(const device_property_t* prop, cJSON* json_value, property_value_t* out_value) {
    if (!prop || !json_value || !out_value) return false;

    switch (prop->type) {
    case PROP_TYPE_SWITCH:
        if (cJSON_IsBool(json_value)) {
            out_value->b = cJSON_IsTrue(json_value);
            return true;
        }
        break;
    case PROP_TYPE_SLIDER:
    case PROP_TYPE_ENUM:
        if (cJSON_IsNumber(json_value)) {
            out_value->i = json_value->valueint;
            return true;
        }
        break;
    // 其他类型转换...
    default:
        break;
    }
    return false;
}
#endif


#if useoldfunc
void Cloud_dispatcher_process_command(const char* payload_json) {
#else
void Cloud_dispatcher_process_command(const char* deviceID, const char* payload_json){
#endif
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
    const char* cmd_id = (id_item && id_item->valuestring) ? id_item->valuestring : "unknown";

    // 获取 params 对象
    cJSON* params = cJSON_GetObjectItem(root, "params");
    if (!params) {
        cJSON_Delete(root);
        // 即使没有params，也应该回复
        MQTT_send_reply(cmd_id, 200, "success");
        return;
    }

    // 遍历 params 对象中的所有条目(功能点标识符)
    cJSON* current_param = params->child;
    while (current_param != NULL) {
        const char* propID = current_param->string; // "LED", "angle"等功能点

#if useoldfunc
        // 在注册表中查找匹配的处理器
        bool handled = false;
        for (int i = 0; i < handler_count; ++i) {
            if (strcmp(handler_registry[i].identifier, propID) == 0) {
                // 调用匹配处理
                handler_registry[i].handler(current_param);
                handled = true;
                break;
            }
        }
        if (!handled) {
            printf("Warning: No handler for identifier '%s'\r\n", propID);
        }
        current_param = current_param->next; // 移动到下一个参数
    }

#else
        // 从DeviceManager查询此属性的预期类型
        const device_property_t* prop_info = DeviceManager_GetProperty(deviceID, propID);
        if (!prop_info) {
        	printf("[Cloud] Warning: No property '%s' found for device '%s'\r\n", propID, deviceID);
            current_param = current_param->next;
            continue;
        }

        // 转换数据类型
        property_value_t new_value;
        if (convert_cjson_to_prop_value(prop_info, current_param, &new_value)) {
        	// 调用DeviceManager更新属性
        	DeviceManager_UpdateProperty(deviceID, propID, new_value);
        }
        else {
        	printf("[Warning]Cloud_dispatcher: Type mismatch for property '%s'\r\n", propID);
        }
        	current_param = current_param->next;
	}
#endif

    cJSON_Delete(root);// 清理JSON对象
    // 立即构建并发送成功回复 (这个函数将在 esp_mqtt.c 中实现)
    MQTT_send_reply(cmd_id, 200, "success");
}


