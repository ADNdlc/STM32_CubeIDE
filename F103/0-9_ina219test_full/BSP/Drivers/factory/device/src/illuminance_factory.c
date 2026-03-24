/*
 * illuminance_factory.c
 *
 *  Created on: Feb 11, 2026
 *      Author: Antigravity
 *
 *  光照传感器工厂实现
 */

#include "elog.h"
#define LOG_TAG "ILLUMINANCE_FACTORY"
#include "i2c_factory.h"
#include "illuminance_factory.h"
#include <stddef.h>

#if (DEV_LIGHT == USE_BH1750)
#include "bh1750/bh1750_driver.h"
#endif

// 存储已创建的光照传感器驱动实例指针数组
static illuminance_driver_t *illuminance_drivers[LIGHT_SENSOR_MAX] = {NULL};

illuminance_driver_t *illuminance_driver_get(light_sensor_id_t id) {
  if (id >= LIGHT_SENSOR_MAX) {
    return NULL;
  }

  if (illuminance_drivers[id] == NULL) {
    const light_sensor_mapping_t *mapping = &light_sensor_mappings[id];

#if (DEV_LIGHT == USE_BH1750)
    // BH1750 依赖 i2c_driver
    // 这里的 mapping->resource 存放的是 i2c_device_id_t 索引
    i2c_driver_t *i2c_drv = i2c_driver_get((i2c_device_id_t)mapping->resource);
    if (!i2c_drv) {
      log_e("Failed to get i2c driver!");
      return NULL;
    }
    illuminance_drivers[id] = bh1750_driver_create(i2c_drv);
#endif
  }

  return illuminance_drivers[id];
}
