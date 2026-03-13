#ifndef BSP_FACTORY_SERVICE_WIFI_FACTORY_H_
#define BSP_FACTORY_SERVICE_WIFI_FACTORY_H_

#include "service_id.h"
#include "Service/wifi_driver.h"

/**
 * @brief 通过逻辑 ID 获取 WiFi 驱动实例
 * @param id 逻辑 ID
 * @return wifi_driver_t* 驱动实例指针
 */
wifi_driver_t *wifi_driver_get(wifi_id_t id);

#endif /* BSP_FACTORY_SERVICE_WIFI_FACTORY_H_ */
