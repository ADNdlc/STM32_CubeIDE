#include "cJSON.h"
#include "esp_app/esp_mqtt/esp_mqtt.h"
#include "device_manager.h"
#include "stdio.h"
#include "string.h"

// 用于限制同步到云端的频率，避免频繁更新
#define SYNC_INTERVAL_MS 1000

// 记录上次同步时间
static uint32_t last_sync_time = 0;

// 标志位，用于防止在处理云命令时触发状态同步
static bool is_processing_cloud_command = false;

/* 接收云命令
 * payload_json: 形如 {"id":"12","version":"1.0","params":{"LED":true}}
 */
void device_manager_process_command(const char* payload_json){
    is_processing_cloud_command = true; // 设置标志位
    
    cJSON* root = cJSON_Parse(payload_json);
    if (root == NULL) {
        printf("Error: Failed to parse JSON payload.\r\n");
        // 即使解析失败，也尝试回复避免超时
        MQTT_send_reply("unknown", 400, "json parse error");
        is_processing_cloud_command = false; // 清除标志位
        return;
    }
    
    // 提取重要的元数据 (id)
    cJSON* id_item = cJSON_GetObjectItemCaseSensitive(root, "id");
    char cmd_id[32] = "unknown"; // 使用固定大小的缓冲区
    if (cJSON_IsString(id_item) && (id_item->valuestring != NULL)) {
        strncpy(cmd_id, id_item->valuestring, sizeof(cmd_id) - 1);
        cmd_id[sizeof(cmd_id) - 1] = '\0'; // 确保字符串结束
    }
    
    // 获取 params 对象
    cJSON* params = cJSON_GetObjectItemCaseSensitive(root, "params");
    if (!params) {
        cJSON_Delete(root);
        // 即使没有params，也应该回复
        MQTT_send_reply(cmd_id, 200, "success");
        is_processing_cloud_command = false; // 清除标志位
        return;
    }
    
    // 遍历 params 中的每个属性
    cJSON* current_param = params->child;
    while (current_param != NULL) {
        const char* propID = current_param->string;
        // 遍历所有设备查找属性
        uint8_t device_count = DeviceManager_GetDeviceCount();
        for (int i = 0; i < device_count; i++) {
            const device_data_t* device = DeviceManager_GetDeviceByIndex(i);
            if (!device) continue;
            // 检查设备是否包含此属性
            const device_property_t* prop = NULL;
            for (int j = 0; j < device->property_count; j++) {
                if (strcmp(device->properties[j].id, propID) == 0) {
                    prop = &device->properties[j];
                    break;
                }
            }
            // 如果找到了属性，则更新它
            if (prop) {
                property_value_t new_value;
                bool value_valid = false;
                
                // 根据属性类型解析值
                switch (prop->type) {
                    case PROP_TYPE_SWITCH:
                        if (cJSON_IsBool(current_param)) {
                            new_value.b = cJSON_IsTrue(current_param);
                            value_valid = true;
                        }
                        break;
                        
                    case PROP_TYPE_SLIDER:
                    case PROP_TYPE_ENUM:
                        if (cJSON_IsNumber(current_param)) {
                            new_value.i = current_param->valueint;
                            value_valid = true;
                        }
                        break;
                        
                    case PROP_TYPE_READONLY:
                        if (cJSON_IsString(current_param)) {
                            new_value.s = current_param->valuestring;
                            value_valid = true;
                        }
                        break;
                        
                    default:
                        break;
                }

                // 如果值有效，则更新属性
                if (value_valid) {
                    DeviceManager_UpdateProperty(device->deviceID, propID, new_value);
                }
                break; // 找到设备并处理后跳出循环
            }
        }
        current_param = current_param->next;
    }
    cJSON_Delete(root);
    
    // 发送回复消息，避免云端超时
    MQTT_send_reply(cmd_id, 200, "success");
    
    is_processing_cloud_command = false; // 清除标志位
}

/* 将设备管理器中的设备状态同步给云端(观察者回调)
 *
 */
void device_manager_state_sync(const device_data_t* device, const char* prop_id){
    // 检查是否正在处理云命令，如果是则跳过同步
    if (is_processing_cloud_command) {
        return;
    }
    
    // 获取当前系统时间
    uint32_t current_time = HAL_GetTick();
    
    // 检查距离上次同步的时间是否足够长
    if ((current_time - last_sync_time < SYNC_INTERVAL_MS) && (last_sync_time != 0)) {
        // 时间间隔太短，跳过同步
        return;
    }
    
    // 更新上次同步时间
    last_sync_time = current_time;
    
    // 获取属性信息
    const device_property_t* prop = NULL;
    for (int i = 0; i < device->property_count; i++) {
        if (strcmp(device->properties[i].id, prop_id) == 0) {
            prop = &device->properties[i];
            break;
        }
    }
    
    if (!prop) {
        return;
    }
    
    // 构造JSON格式的payload
    cJSON* root = cJSON_CreateObject();
    if (!root) return;
    
    cJSON* params = cJSON_CreateObject();
    if (!params) {
        cJSON_Delete(root);
        return;
    }
    
    cJSON_AddItemToObject(root, "params", params);
    
    // 根据属性类型添加值到JSON
    switch (prop->type) {
        case PROP_TYPE_SWITCH:
            cJSON_AddBoolToObject(params, prop_id, prop->value.b);
            break;
            
        case PROP_TYPE_SLIDER:
        case PROP_TYPE_ENUM:
            cJSON_AddNumberToObject(params, prop_id, prop->value.i);
            break;
            
        case PROP_TYPE_READONLY:
            cJSON_AddStringToObject(params, prop_id, prop->value.s);
            break;
            
        default:
            cJSON_Delete(root);
            return;
    }
    
    // 转换为字符串
    char* payload_str = cJSON_PrintUnformatted(root);
    if (!payload_str) {
        cJSON_Delete(root);
        return;
    }
    
    // 构造topic并发布
    char topic[128];
    snprintf(topic, sizeof(topic), "$sys/%s/%s/thing/property/post", "SQKg9n0Ii0", device->deviceID);
    
    MQTT_publish(topic, payload_str, 0, 0);
    
    // 清理内存
    cJSON_free(payload_str);
    cJSON_Delete(root);
}