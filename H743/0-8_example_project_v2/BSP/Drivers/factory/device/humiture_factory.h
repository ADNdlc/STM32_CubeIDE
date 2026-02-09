/*
 * thsensor_factory.h
 *
 *  Created on: Feb 9, 2026
 *      Author: Antigravity
 *
 *  温湿度传感器工厂层
 */

#ifndef BSP_DRIVERS_FACTORY_SENSOR_THSENSOR_FACTORY_H_
#define BSP_DRIVERS_FACTORY_SENSOR_THSENSOR_FACTORY_H_

#include "humiture_driver.h"
#include "dev_map.h"

/**
 * @brief 获取温湿度传感器驱动实例
 * @param id 传感器逻辑 ID
 * @return 传感器驱动指针，失败返回 NULL
 */
humiture_driver_t *humiture_driver_get(th_sensor_id_t id);

#endif /* BSP_DRIVERS_FACTORY_SENSOR_THSENSOR_FACTORY_H_ */
