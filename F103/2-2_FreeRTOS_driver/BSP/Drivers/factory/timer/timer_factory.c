/*
 * timer_factory.c
 *
 *  Created on: Feb 20, 2026
 *      Author: 12114
 */

#ifndef DRIVERS_FACTORY_TIMER_TIMER_FACTORY_C_
#define DRIVERS_FACTORY_TIMER_TIMER_FACTORY_C_

#include "timer_factory.h"
#include <stddef.h>

#if (TARGET_PLATFORM == PLATFORM_STM32)
#include "timer/stm32_timer_driver.h"
#endif

static timer_driver_t *timer_drivers[TIMER_ID_MAX] = {NULL};

timer_driver_t *timer_driver_get(timer_device_id_t id) {
  if (id >= TIMER_ID_MAX) {
    return NULL;
  }

  if (timer_drivers[id] == NULL) {
    const timer_mapping_t *mapping = &timer_mappings[id];
    if (mapping->resource == NULL) {
      return NULL;
    }

#if (TARGET_PLATFORM == PLATFORM_STM32)
    timer_drivers[id] =
        stm32_timer_driver_create((TIM_HandleTypeDef *)mapping->resource);
#endif
  }

  return timer_drivers[id];
}

#endif /* DRIVERS_FACTORY_TIMER_TIMER_FACTORY_C_ */
