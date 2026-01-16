/*
 * dht11_driver.h
 *
 *  Created on: Jan 16, 2026
 *      Author: Antigravity
 *
 *  DHT11 传感器驱动
 */

#ifndef DEVICE_DHT11_DHT11_DRIVER_H_
#define DEVICE_DHT11_DHT11_DRIVER_H_

#include "one_wire_driver.h"
#include <stdint.h>

// DHT11 数据结构
typedef struct {
  one_wire_driver_t *ow_drv; // 单总线驱动
  uint8_t humidity;          // 湿度 (%)
  uint8_t temperature;       // 温度 (℃)
} dht11_driver_t;

/**
 * @brief 初始化 DHT11 驱动
 * @param self 驱动实例指针
 * @param ow_drv 单总线驱动实例指针
 * @return 0: 成功, !0: 失败
 */
int dht11_init(dht11_driver_t *self, one_wire_driver_t *ow_drv);

/**
 * @brief 读取温湿度数据
 * @param self 驱动实例指针
 * @return 0: 成功, !0: 失败
 */
int dht11_read_data(dht11_driver_t *self);

#endif /* DEVICE_DHT11_DHT11_DRIVER_H_ */
