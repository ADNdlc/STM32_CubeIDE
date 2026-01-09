#ifndef APPLICATION_HOME_SYSTEM_CLOUD_BRIDGE_H_
#define APPLICATION_HOME_SYSTEM_CLOUD_BRIDGE_H_

#include "mqtt_service.h"
#include "thing_model.h"

/**
 * @brief 初始化云桥负责物模型的云同步
 *
 * @param mqtt_svc MQTT服务实例
 */
void cloud_bridge_init(mqtt_service_t *mqtt_svc);

/**
 * @brief 处理物模型云同步
 */
void cloud_bridge_process(void);

#endif /* APPLICATION_HOME_SYSTEM_CLOUD_BRIDGE_H_ */
