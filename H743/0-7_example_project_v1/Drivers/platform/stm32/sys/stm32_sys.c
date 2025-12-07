/*
 * stm32_sys.c
 *
 *  Created on: Dec 5, 2025
 *      Author: 12114
 */

#include "stm32_sys.h"
#include "main.h"
#include "sys_hal/sys_hal.h"

static uint32_t stm32_get_tick(void) { return HAL_GetTick(); }

static void stm32_delay(uint32_t ms) { HAL_Delay(ms); }

static const sys_driver_t stm32_sys_driver = {.get_tick = stm32_get_tick,
                                              .delay = stm32_delay};

void stm32_sys_init(void) { sys_hal_init(&stm32_sys_driver); }
