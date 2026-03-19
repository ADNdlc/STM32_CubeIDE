/*
 * storage_factory.h
 *
 *  Created on: Mar 19, 2026
 *      Author: Antigravity
 */

#ifndef DRIVERS_FACTORY_DEVICE_STORAGE_FACTORY_H_
#define DRIVERS_FACTORY_DEVICE_STORAGE_FACTORY_H_

#include "storage_interface.h"
#include "dev_map.h"


storage_device_t *storage_factory_get(storage_device_id_t id);

/**
 * @brief 创建 各种类型的 存储设备实例
 * @param map->resource 	底层资源
 * @param map->type 		具体调用哪种设备工厂
 * @return storage_device_t* 存储设备接口指针，失败返回 NULL
 */
storage_device_t *storage_factory_create(const storage_mapping_t *map);

#endif /* DRIVERS_FACTORY_DEVICE_STORAGE_FACTORY_H_ */