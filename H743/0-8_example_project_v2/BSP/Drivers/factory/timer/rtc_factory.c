/*
 * rtc_factory.c
 *
 *  Created on: Feb 13, 2026
 *      Author: Antigravity
 */

#include "rtc_factory.h"
#include <stddef.h>

#if (TARGET_PLATFORM == PLATFORM_STM32)
#include "rtc/stm32_rtc_driver.h"
#endif

static rtc_driver_t *rtc_drivers[RTC_MAX] = {NULL};

rtc_driver_t *rtc_driver_get(rtc_device_id_t id) {
  if (id >= RTC_MAX) {
    return NULL;
  }

  if (rtc_drivers[id] == NULL) {
    const rtc_mapping_t *mapping = &rtc_mappings[id];
    if (mapping->resource == NULL) {
      return NULL;
    }

#if (TARGET_PLATFORM == PLATFORM_STM32)
    rtc_drivers[id] =
        stm32_rtc_driver_create((RTC_HandleTypeDef *)mapping->resource);
#endif
  }

  return rtc_drivers[id];
}
