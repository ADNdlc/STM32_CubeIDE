/*
 * mqtt_port.h
 *
 *  Created on: Nov 12, 2025
 *      Author: 12114
 */

#ifndef LVGL_APP_SERVICE_MQTT_PORT_H_
#define LVGL_APP_SERVICE_MQTT_PORT_H_

/**
 * @brief 兼容旧版本，将云命令解析中调用将"params"发给DeviceManager进行处理
 * @param payload_json 命令的JSON字符串
 */
void device_manager_process_command(const char *payload_json);

/* 将设备管理器中的设备状态同步给云端(观察者回调)
 *
 */
void device_manager_state_sync(const device_data_t* device, const char* prop_id);

#endif /* LVGL_APP_SERVICE_MQTT_PORT_H_ */
