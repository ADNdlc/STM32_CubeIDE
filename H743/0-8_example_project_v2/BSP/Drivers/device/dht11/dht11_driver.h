/*
 * dht11_driver.h
 *
 *  Created on: Jan 16, 2026
 *      Author: Antigravity
 *
 *  DHT11 传感器驱动 (支持 thsensor_driver 接口)
 */

#ifndef DEVICE_DHT11_DHT11_DRIVER_H_
#define DEVICE_DHT11_DHT11_DRIVER_H_

#include "humiture_driver.h"
#include "one_wire_driver.h"
#include <stdint.h>


// DHT11 驱动结构体
typedef struct {
  humiture_driver_t base;    // 基类接口
  one_wire_driver_t *ow_drv; // 依赖的单总线驱动
  uint8_t humidity;          // 缓存的湿度 (%)
  uint8_t temperature;       // 缓存的温度 (℃)
} dht11_driver_t;

/**
 * @brief 创建 DHT11 驱动实例 (适配 thsensor_driver 接口)
 * @param ow_drv 单总线驱动实例指针
 * @return 抽象驱动指针
 */
humiture_driver_t *dht11_driver_create(one_wire_driver_t *ow_drv);

#endif /* DEVICE_DHT11_DHT11_DRIVER_H_ */
