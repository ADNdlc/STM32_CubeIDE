/*
 * illuminance_factory.h
 *
 *  Created on: Feb 11, 2026
 *      Author: Antigravity
 *
 *  光照传感器工厂层
 */

#ifndef BSP_DRIVERS_FACTORY_DEVICE_ILLUMINANCE_FACTORY_H_
#define BSP_DRIVERS_FACTORY_DEVICE_ILLUMINANCE_FACTORY_H_

#include "illuminance_driver.h"
#include "dev_map.h"

/**
 * @brief 获取光照传感器驱动实例
 * @param id 传感器逻辑 ID
 * @return 传感器驱动指针，失败返回 NULL
 */
illuminance_driver_t *illuminance_driver_get(light_sensor_id_t id);

#endif /* BSP_DRIVERS_FACTORY_DEVICE_ILLUMINANCE_FACTORY_H_ */
