/*
 * qspi_factory.h
 *
 *  Created on: Feb 16, 2026
 *      Author: Antigravity
 */

#ifndef BSP_DRIVERS_FACTORY_CONNECTIVITY_QSPI_FACTORY_H_
#define BSP_DRIVERS_FACTORY_CONNECTIVITY_QSPI_FACTORY_H_

#include "dev_map.h"
#include "qspi_driver.h"

/**
 * @brief 获取 QSPI 驱动实例
 * @param id QSPI 设备逻辑 ID
 * @return qspi_driver_t* 驱动实例指针
 */
qspi_driver_t *qspi_driver_get(qspi_device_id_t id);

#endif /* BSP_DRIVERS_FACTORY_CONNECTIVITY_QSPI_FACTORY_H_ */
