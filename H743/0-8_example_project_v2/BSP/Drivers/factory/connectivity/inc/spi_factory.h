/*
 * spi_factory.h
 *
 *  Created on: Feb 16, 2026
 *      Author: Antigravity
 */

#ifndef BSP_DRIVERS_FACTORY_CONNECTIVITY_SPI_FACTORY_H_
#define BSP_DRIVERS_FACTORY_CONNECTIVITY_SPI_FACTORY_H_

#include "dev_map.h"
#include "spi_driver.h"

/**
 * @brief 获取 SPI 驱动实例
 * @param id SPI 设备逻辑 ID
 * @return spi_driver_t* 驱动实例指针
 */
spi_driver_t *spi_driver_get(spi_device_id_t id);

#endif /* BSP_DRIVERS_FACTORY_CONNECTIVITY_SPI_FACTORY_H_ */
