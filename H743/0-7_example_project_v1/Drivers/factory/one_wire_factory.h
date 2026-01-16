/*
 * one_wire_factory.h
 *
 *  Created on: Jan 16, 2026
 *      Author: Antigravity
 *
 *  One-Wire 驱动工厂
 */

#ifndef FACTORY_ONE_WIRE_FACTORY_H_
#define FACTORY_ONE_WIRE_FACTORY_H_

#include "device_mapping.h"
#include "one_wire_driver.h"

/**
 * @brief 获取 One-Wire 软件模拟驱动实例
 * @param id One-Wire 软件模拟设备 ID
 * @return One-Wire 驱动实例指针，失败返回 NULL
 */
one_wire_driver_t *one_wire_soft_driver_get(one_wire_soft_device_id_t id);

#endif /* FACTORY_ONE_WIRE_FACTORY_H_ */
