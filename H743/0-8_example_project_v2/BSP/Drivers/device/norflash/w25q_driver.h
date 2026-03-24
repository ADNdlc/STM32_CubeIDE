/*
 * w25q_driver.h
 *
 *  Created on: Feb 19, 2026
 *      Author: Antigravity
 */

#ifndef BSP_DRIVERS_DEVICE_NORFLASH_W25Q_DRIVER_H_
#define BSP_DRIVERS_DEVICE_NORFLASH_W25Q_DRIVER_H_

#include "nor_flash_driver.h"
#include "qspi_driver.h"
#include "sys.h"

typedef struct {
  nor_flash_driver_t base;
  qspi_driver_t *qspi;
} w25q_driver_t;

/**
 * @brief 创建 W25Q 驱动实例
 * @param qspi QSPI 总线驱动
 * @return 驱动指针
 */
nor_flash_driver_t *w25q_driver_create(qspi_driver_t *qspi, nor_flash_info_t *info);

#endif /* BSP_DRIVERS_DEVICE_NORFLASH_W25Q_DRIVER_H_ */
