/*
 * rtc_factory.h
 *
 *  Created on: Feb 13, 2026
 *      Author: Antigravity
 */

#ifndef BSP_DRIVERS_FACTORY_CONNECTIVITY_RTC_FACTORY_H_
#define BSP_DRIVERS_FACTORY_CONNECTIVITY_RTC_FACTORY_H_

#include "dev_map.h"
#include "rtc_driver.h"


/**
 * @brief 获取 RTC 驱动实例
 * @param id RTC 逻辑 ID
 * @return 驱动指针
 */
rtc_driver_t *rtc_driver_get(rtc_device_id_t id);

#endif /* BSP_DRIVERS_FACTORY_CONNECTIVITY_RTC_FACTORY_H_ */
