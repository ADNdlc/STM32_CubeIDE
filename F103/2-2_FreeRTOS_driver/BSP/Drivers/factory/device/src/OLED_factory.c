/*
 * OLED_factory.c
 *
 *  Created on: Mar 3, 2026
 *      Author: Antigravity
 */

#include "OLED_factory.h"
#include "i2c_factory.h"
#include "spi_factory.h"
#include "gpio_factory.h"
//#include "MemPool.h"
#include <stdlib.h>
#include <string.h>


// 静态实例缓存
static oled_device_t *oled_drivers[OLED_ID_MAX] = {NULL};

oled_device_t *OLED_Factory_Get(oled_id_t id) {
  if (id >= OLED_ID_MAX)
    return NULL;

  if (oled_drivers[id] == NULL) {
    const oled_mapping_t *oled_mapping = &oled_mappings[id];
    oled_config_t *config = (oled_config_t *)oled_mapping->resource;

    if (!config)
      return NULL;

    // 1. 创建总线
    oled_bus_t *bus = NULL;
    if (config->bus_type == OLED_BUS_I2C) {
      i2c_driver_t *i2c_drv = i2c_driver_get(config->oled_i2c_config.i2c_id);
      if (i2c_drv) {
        bus = OLED_I2C_Bus_Create(i2c_drv, &config->oled_i2c_config);
      }
    } else if (config->bus_type == OLED_BUS_SPI) {
      bus_drivers_t bus_drivers;
      bus_drivers.spi_dev = spi_driver_get(config->oled_spi_config.spi_id);
      bus_drivers.dc_pin = gpio_driver_get(config->oled_spi_config.dc_pin);
      bus_drivers.res_pin = gpio_driver_get(config->oled_spi_config.res_pin);

      if (bus_drivers.spi_dev && bus_drivers.dc_pin) {
        bus = OLED_SPI_Bus_Create(&bus_drivers, &config->oled_spi_config);
      }
    }

    if (!bus)
      return NULL;

    // 2. 创建设备实体
    oled_device_t *dev =
        (oled_device_t *)sys_malloc(SYS_MEM_INTERNAL, sizeof(oled_device_t));
    if (dev) {
      dev->ops = config->ops;
      dev->bus = bus;
      dev->width = config->width;
      dev->height = config->height;
      dev->col_offset = config->col_offset;
      dev->buffer = config->buffer;

      oled_drivers[id] = dev;
    }
  }
  return oled_drivers[id];
}
