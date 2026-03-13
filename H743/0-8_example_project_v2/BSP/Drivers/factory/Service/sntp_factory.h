#ifndef BSP_FACTORY_SERVICE_SNTP_FACTORY_H_
#define BSP_FACTORY_SERVICE_SNTP_FACTORY_H_

#include "service_id.h"
#include "Service/sntp_driver.h"

/**
 * @brief 通过逻辑 ID 获取 SNTP 驱动实例
 * @param id 逻辑 ID
 * @return sntp_driver_t* 驱动实例指针
 */
sntp_driver_t *sntp_driver_get(sntp_id_t id);

#endif /* BSP_FACTORY_SERVICE_SNTP_FACTORY_H_ */
