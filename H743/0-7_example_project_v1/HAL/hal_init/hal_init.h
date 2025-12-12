/*
 * hal_init.h
 *
 *  Created on: Dec 12, 2025
 *      Author: 12114
 */

#ifndef APPLICATION_HAL_INIT_HAL_INIT_H_
#define APPLICATION_HAL_INIT_HAL_INIT_H_

#define LVGL_INIT 0

#include "SYSTEM/sys.h"
#include "all_tests_config.h"
#include "factory.h"
#include "logger/elog_init.h"
#if LVGL_INIT
#include "lv_port_disp.h"
#include "lvgl.h"
#endif

int hal_init(void);


#endif /* APPLICATION_HAL_INIT_HAL_INIT_H_ */
