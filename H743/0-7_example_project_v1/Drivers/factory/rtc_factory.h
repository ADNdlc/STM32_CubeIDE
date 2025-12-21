/*
 * rtc_factory.h
 *
 *  Created on: Dec 21, 2025
 *      Author: 12114
 */

#ifndef DRIVERS_FACTORY_RTC_FACTORY_H_
#define DRIVERS_FACTORY_RTC_FACTORY_H_

#include "device_mapping.h"
#include "rtc_driver.h"
#include "stm32h7xx_hal.h"

rtc_driver_t *rtc_driver_get(rtc_device_id_t id);

#endif /* DRIVERS_FACTORY_RTC_FACTORY_H_ */
