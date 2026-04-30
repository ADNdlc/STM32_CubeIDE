/*
 * bh1750_driver.h
 *
 *  Created on: Feb 11, 2026
 *      Author: Antigravity
 *
 *  BH1750 光照传感器驱动 (I2C)
 */

#ifndef BSP_DRIVERS_DEVICE_BH1750_BH1750_DRIVER_H_
#define BSP_DRIVERS_DEVICE_BH1750_BH1750_DRIVER_H_

#include "../../interface/Connectivity/i2c_driver.h"
#include "../../interface/device/illuminance_driver.h"


// BH1750 驱动结构体
typedef struct {
  illuminance_driver_t base; // 基类
  i2c_driver_t *i2c_drv;     // 依赖的 I2C 驱动
  uint16_t dev_addr;         // I2C 设备地址
} bh1750_driver_t;

/**
 * @brief 创建 BH1750 驱动实例
 * @param i2c_drv I2C 驱动实例
 * @return 光照传感器驱动指针
 */
illuminance_driver_t *bh1750_driver_create(i2c_driver_t *i2c_drv);

#endif /* BSP_DRIVERS_DEVICE_BH1750_BH1750_DRIVER_H_ */
