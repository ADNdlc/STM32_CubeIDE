#ifndef BSP_FACTORY_SERVICE_MQTT_FACTORY_H_
#define BSP_FACTORY_SERVICE_MQTT_FACTORY_H_

#include "service_id.h"
#include "mqtt_driver.h"

/**
 * @brief 通过逻辑 ID 获取 MQTT 驱动实例
 * @param id 逻辑 ID
 * @return mqtt_driver_t* 驱动实例指针
 */
mqtt_driver_t *mqtt_driver_get(mqtt_id_t id);

#endif /* BSP_FACTORY_SERVICE_MQTT_FACTORY_H_ */
