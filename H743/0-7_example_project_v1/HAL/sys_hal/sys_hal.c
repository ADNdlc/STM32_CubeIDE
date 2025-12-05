/*
 * sys_hal.c
 *
 *  Created on: Dec 5, 2025
 *      Author: 12114
 */

#include "sys_hal.h"
#include <stddef.h>

static const sys_driver_t *g_sys_driver = NULL;

void sys_hal_init(const sys_driver_t *driver) { g_sys_driver = driver; }

uint32_t sys_hal_get_tick(void) {
  if (g_sys_driver && g_sys_driver->get_tick) {
    return g_sys_driver->get_tick();
  }
  return 0;
}

void sys_hal_delay(uint32_t ms) {
  if (g_sys_driver && g_sys_driver->delay) {
    g_sys_driver->delay(ms);
  }
}
