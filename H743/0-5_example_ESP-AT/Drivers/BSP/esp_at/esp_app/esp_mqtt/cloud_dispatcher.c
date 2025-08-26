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

#define MAX_CLOUD_HANDLERS 20 // 定义最大可注册的命令数量(可写功能点数量)
extern void MQTT_send_reply(const char* id, int code, const char* msg);

// 命令注册表条目
typedef struct {
    const char* identifier;		//功能点标识符
    cloud_cmd_handler_t handler;//处理函数
} Cloud_Handler_t;

// 命令注册表实例
static Cloud_Handler_t handler_registry[MAX_CLOUD_HANDLERS];// 云命令处理表
static int handler_count = 0;								// 已有条目数

/*	@brief	云命令处理项添加
 * 			设备使用此api注册云属性设置回调
 *
 *	@param	identifier	功能点标识符
 *	@param	handler		处理函数
 *
 *	@return	执行结果
 */
bool Cloud_dispatcher_register(const char* identifier, cloud_cmd_handler_t handler) {
    if (handler_count >= MAX_CLOUD_HANDLERS) {
        return false; // 表已满
    }
    handler_registry[handler_count].identifier = identifier;
    handler_registry[handler_count].handler = handler;
    handler_count++;
    return true;
}

/*	@brief	云命令分发器
 * 			遍历handler_registry中所有处理项调用相应处理函数
 *
 *	@param	payload_json  云命令内容中提取出的Json载荷

 */
void Cloud_dispatcher_process_command(const char* payload_json) {
    cJSON* root = cJSON_Parse(payload_json);
    if (root == NULL) {
#ifndef NDEBUG
        printf("Cloud_process:Fail parse JSON!!!\r\n");
#endif
        return;
    }
    // 提取元数据
    cJSON* id_item = cJSON_GetObjectItem(root, "id");
    if (id_item == NULL || !cJSON_IsString(id_item)) {
        cJSON_Delete(root);return;
    }
    const char* cmd_id = id_item->valuestring;
    // 获取 params 对象
    cJSON* params = cJSON_GetObjectItem(root, "params");
    if (params == NULL || !cJSON_IsObject(params)) {
        cJSON_Delete(root);
        // 没有params
#ifndef  NDEBUG
        printf("JSON is NULL\r\n");
#endif
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
            	/* 调用相应处理 */
                handler_registry[i].handler(current_param);
                handled = true;
                break;
            }
        }
        if (!handled) {
#ifndef NDEBUG
        printf("Cloud_process:No handler '%s'\r\n", identifier);
#endif
        }
        current_param = current_param->next; // 移动到下一个参数
    }
    // 立即构建并发送成功回复 (这个函数将在 esp_mqtt.c 中实现)
    MQTT_send_reply(cmd_id, 200, "success");
    cJSON_Delete(root);// 最后清理JSON对象,传入的是地址
}

