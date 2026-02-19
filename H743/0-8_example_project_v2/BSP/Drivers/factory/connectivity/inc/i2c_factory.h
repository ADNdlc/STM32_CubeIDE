/*
 * i2c_factory.h
 *
 *  Created on: Feb 11, 2026
 *      Author: Antigravity
 *
 *  I2C 驱动工厂层
 */

#ifndef BSP_DRIVERS_FACTORY_CONNECTIVITY_I2C_FACTORY_H_
#define BSP_DRIVERS_FACTORY_CONNECTIVITY_I2C_FACTORY_H_

#include "i2c_driver.h"
#include "dev_map.h"

/**
 * @brief 获取 I2C 驱动实例
 * @param id I2C 总线逻辑 ID
 * @return I2C 驱动指针，失败返回 NULL
 */
i2c_driver_t *i2c_driver_get(i2c_device_id_t id);

#endif /* BSP_DRIVERS_FACTORY_CONNECTIVITY_I2C_FACTORY_H_ */
