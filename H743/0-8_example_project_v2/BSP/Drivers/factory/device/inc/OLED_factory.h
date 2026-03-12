/*
 * OLED_factory.h
 *
 *  Created on: Mar 3, 2026
 *      Author: Antigravity
 */

#ifndef DRIVERS_FACTORY_DEVICE_INC_OLED_FACTORY_H_
#define DRIVERS_FACTORY_DEVICE_INC_OLED_FACTORY_H_

#include "dev_map.h"
#include "oled/bus/oled_i2c_bus.h"
#include "oled/bus/oled_spi_bus.h"
#include "oled_driver.h"


typedef enum {
  OLED_BUS_I2C = 0,
  OLED_BUS_SPI,
} oled_bus_type_t;

typedef struct oled_config_t {
  const oled_chip_ops_t *ops; // 无依赖资源，不使用工厂创建，直接绑定函数表
  oled_bus_type_t bus_type;   // 依赖对应总线驱动，使用对应总线工厂获取
  union {
    oled_spi_config_t oled_spi_config;
    oled_i2c_config_t oled_i2c_config;
  };
  uint16_t width;
  uint16_t height;
  uint8_t col_offset;
  uint8_t *buffer;
} oled_config_t;

oled_device_t *OLED_Factory_Get(oled_id_t id);

#endif /* DRIVERS_FACTORY_DEVICE_INC_OLED_FACTORY_H_ */
