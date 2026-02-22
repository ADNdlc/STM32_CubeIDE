/*
 * PowerMonitor_factory.c
 *
 *  Created on: Feb 22, 2026
 *      Author: 12114
 */

#line 7
#include "PowerMonitor_factory.h"
#include "dev_map.h"
#include <stddef.h>

#if (DEV_POWER_MONITOR == USE_INA219)
#include "i2c_factory.h"
#include "ina219/ina219_driver.h"
#include "timer_factory.h"
#endif

static PowerMonitor_driver_t *power_monitor_drivers[POWER_MONITOR_MAX] = {NULL};

PowerMonitor_driver_t *PowerMonitor_factory_get(power_monitor_id_t id) {
  if (id >= POWER_MONITOR_MAX) {
    return NULL;
  }

  if (power_monitor_drivers[id] == NULL) {
    const power_monitor_mapping_t *power_monitor_mapping =
        &power_monitor_mappings[id];
#if (DEV_POWER_MONITOR == USE_INA219)
    // 转换为ina219配置
    ina219_factory_config_t *ina219_factory_config =
        (ina219_factory_config_t *)power_monitor_mapping->resource;
    // 获取i2c驱动
    i2c_driver_t *i2c_driver = i2c_driver_get(ina219_factory_config->i2c_id);
    if (i2c_driver == NULL) {
      return NULL;
    }
    // 获取timer驱动
    timer_driver_t *timer_driver =
        timer_driver_get(ina219_factory_config->timer_id);
    if (timer_driver == NULL) {
      return NULL;
    }
    // 创建ina219驱动
    power_monitor_drivers[id] = ina219_create(i2c_driver, timer_driver,
                                              &(ina219_factory_config->config));

#endif
  }
#line 49
  return power_monitor_drivers[id];
}
