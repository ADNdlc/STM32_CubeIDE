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
#include "gpio_factory.h"
#include "gt9xxx_touch/gt9xxx_touch_driver.h"
#include "i2c_factory.h"


#endif

static touch_driver_t *touch_drivers[TOUCH_MAX] = {NULL};

touch_driver_t *touch_driver_get(touch_id_t id) {
  if (id >= TOUCH_MAX) {
    return NULL;
  }

  if (touch_drivers[id] == NULL) {
    const touch_mapping_t *mapping = &touch_mappings[id];

#if (DEV_TOUCH == USE_GT9XXX)
    gt9xxx_config_t *conf = (gt9xxx_config_t *)mapping->resource;
    gt9xxx_bus_t bus = {
        .i2c = i2c_driver_get(conf->i2c_id),
        .rst_gpio = gpio_driver_get(conf->rst_gpio_id),
        .int_gpio = gpio_driver_get(conf->int_gpio_id),
        .addr_mode = conf->addr_mode,
    };

    touch_drivers[id] = gt9xxx_touch_create(&bus);
    if (touch_drivers[id]) {
      TOUCH_INIT(touch_drivers[id]);
    }
#endif
  }

  return touch_drivers[id];
}
