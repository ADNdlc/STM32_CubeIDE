/*
 * touch_factory.c
 *
 *  Created on: Feb 12, 2026
 *      Author: Antigravity
 *
 *  触摸屏驱动工厂实现
 */

#include "touch_factory.h"
#include <stddef.h>

#if (DEV_TOUCH == USE_GT9XXX)
#include "i2c_factory.h"
#include "gpio_factory.h"
#include "gt9xxx_touch/gt9xxx_touch_driver.h"

#endif

static touch_driver_t *touch_drivers[TOUCH_MAX] = {NULL};

touch_driver_t *touch_driver_get(touch_id_t id) {
  if (id >= TOUCH_MAX) {
    return NULL;
  }

  if (touch_drivers[id] == NULL) {
    const touch_mapping_t *mapping = &touch_mappings[id];

#if (DEV_TOUCH == USE_GT9XXX)
    gt9xxx_bus_t bus = {
        .i2c = i2c_driver_get( CALL_RESOURE(gt9xxx_config_t, i2c_device_id_t, i2c_conf) ),
        .rst_gpio = gpio_driver_get( CALL_RESOURE(gt9xxx_config_t, gpio_device_id_t, rst_gpio_conf) ),
        .int_gpio = gpio_driver_get( CALL_RESOURE(gt9xxx_config_t, gpio_device_id_t, int_gpio_conf) ),
        .addr_mode = ((gt9xxx_config_t*)mapping->resource)->addr_mode_conf,
    };

    touch_drivers[id] = gt9xxx_touch_create(&bus);
    if (touch_drivers[id]) {
      TOUCH_INIT(touch_drivers[id]);
    }
#endif
  }

  return touch_drivers[id];
}
