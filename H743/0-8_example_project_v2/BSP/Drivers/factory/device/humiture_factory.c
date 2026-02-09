/*
 * thsensor_factory.c
 *
 *  Created on: Feb 9, 2026
 *      Author: Antigravity
 *
 *  温湿度传感器工厂层实现
 */

#include <humiture_factory.h>
#include "dht11/dht11_driver.h"
#include <stddef.h>

static thsensor_driver_t *thsensor_drivers[TH_SENSOR_MAX] = {NULL};

thsensor_driver_t *thsensor_driver_get(th_sensor_id_t id) {
  if (id >= TH_SENSOR_MAX) {
    return NULL;
  }

  if (thsensor_drivers[id] == NULL) {
    const th_sensor_mapping_t *mapping = &th_sensor_mappings[id];

    // 根据映射表中的型号创建对应的驱动实例
    switch (mapping->model) {
    case MODEL_DHT11:
      // DHT11 依赖 one_wire_driver
      thsensor_drivers[id] =
          dht11_driver_create((one_wire_driver_t *)mapping->bus_resource);
      break;

    case MODEL_SHT30:
      // TODO: SHT30 依赖 i2c_driver
      break;

    default:
      break;
    }
  }

  return thsensor_drivers[id];
}
