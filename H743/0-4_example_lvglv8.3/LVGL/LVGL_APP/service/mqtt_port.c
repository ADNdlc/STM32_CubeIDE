/*
 * mqtt_port.c
 *
 *  Created on: Nov 12, 2025
 *      Author: 12114
 */

 #include "cJSON.h"
 #include "esp_mqtt.h"

/* 接收云命令
 * payload_json: 形如 {"id":"12","version":"1.0","params":{"LED":true}}
 */
void device_manager_process_command(const char* payload_json){
    cJSON* root = cJSON_Parse(payload_json);



}

/* 将设备管理器中的设备状态同步给云端(观察者回调)
 *
 */
void device_manager_state_sync(const device_data_t* device, const char* prop_id){



}


