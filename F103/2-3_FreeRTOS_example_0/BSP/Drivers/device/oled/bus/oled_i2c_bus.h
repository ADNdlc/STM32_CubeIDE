/*
 * oled_i2c_bus.h
 *
 *  Created on: Mar 2, 2026
 *      Author: 12114
 */

#include "interface_inc.h"
#include "dev_map.h"

// I2C 适配器配置结构体
typedef struct {
  i2c_device_id_t i2c_id;  // I2C 驱动id
  uint16_t     dev_addr;   // OLED 的 I2C 地址 (通常为 0x78 或 0x7A)
} oled_i2c_config_t;

typedef struct {
  oled_bus_t base;
  uint16_t     dev_addr;
  i2c_driver_t* i2c_dev;
} oled_i2c_bus_t;

// 创建 I2C 总线对象的函数
oled_bus_t* OLED_I2C_Bus_Create(i2c_driver_t *i2c, const oled_i2c_config_t * config);
