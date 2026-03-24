/*
 * thsensor_factory.c
 *
 *  Created on: Feb 9, 2026
 *      Author: Antigravity
 *
 *  温湿度传感器工厂层实现
 */

#include <humiture_factory.h>

#include <stddef.h>
#if (DEV_HUMITURE == USE_DHT11)
#include "dht11/dht11_driver.h"
#endif

static humiture_driver_t *humiture_drivers[TH_SENSOR_MAX] = {NULL};

humiture_driver_t *humiture_driver_get(th_sensor_id_t id) {
  if (id >= TH_SENSOR_MAX) {
    return NULL;
  }

  if (humiture_drivers[id] == NULL) {
    const th_sensor_mapping_t *mapping = &th_sensor_mappings[id];
#if (DEV_HUMITURE == USE_DHT11)
    // DHT11 依赖 one_wire_driver
    one_wire_driver_t *ow_drv = one_wire_driver_get((one_wire_device_id_t)mapping->resource);
    humiture_drivers[id] = dht11_driver_create(ow_drv);
#elif (DEV_HUMITURE == USE_DHT22)
    humiture_drivers[id] =
        dht22_driver_create((one_wire_driver_t *)mapping->resource);
#endif
  }

  return humiture_drivers[id];
}
