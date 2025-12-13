/*
 * i2c_factory.h
 *
 *  Created on: Dec 13, 2025
 *      Author: Antigravity
 *
 *  I2C 驱动工厂
 *  根据配置创建对应平台的 I2C 驱动实例
 */

#ifndef FACTORY_I2C_FACTORY_H_
#define FACTORY_I2C_FACTORY_H_

#include "device_mapping.h"
#include "i2c_driver.h"


/**
 * @brief 获取 I2C 软件模拟驱动实例
 * @param id I2C 软件模拟设备 ID
 * @return I2C 驱动实例指针，失败返回 NULL
 */
i2c_driver_t *i2c_soft_driver_get(i2c_soft_device_id_t id);

#endif /* FACTORY_I2C_FACTORY_H_ */
