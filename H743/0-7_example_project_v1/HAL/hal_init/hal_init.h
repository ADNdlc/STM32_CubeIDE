/*
 * hal_init.h
 *
 *  Created on: Dec 12, 2025
 *      Author: 12114
 */

#ifndef APPLICATION_HAL_INIT_HAL_INIT_H_
#define APPLICATION_HAL_INIT_HAL_INIT_H_

#include "project_cfg.h"

#include "SYSTEM/sys.h"
#include "all_tests_config.h"
#include "factory.h"
#include "logger/elog_init.h"
#if LVGL_ENABLE
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lvgl.h"
#endif

int hal_init(void);

#endif /* APPLICATION_HAL_INIT_HAL_INIT_H_ */
