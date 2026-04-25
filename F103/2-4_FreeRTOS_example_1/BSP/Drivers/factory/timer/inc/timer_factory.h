/*
 * timer_factory.h
 *
 *  Created on: Feb 20, 2026
 *      Author: 12114
 */

#ifndef DRIVERS_FACTORY_TIMER_TIMER_FACTORY_H_
#define DRIVERS_FACTORY_TIMER_TIMER_FACTORY_H_

#include "dev_map.h"
#include "interface_inc.h"

/**
 * @brief 获取 RTC 驱动实例
 * @param id RTC 逻辑 ID
 * @return 驱动指针
 */
timer_driver_t *timer_driver_get(timer_device_id_t id);

#endif /* DRIVERS_FACTORY_TIMER_TIMER_FACTORY_H_ */
