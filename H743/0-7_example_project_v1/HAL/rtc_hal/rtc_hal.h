/*
 * rtc_hal.h
 *
 *  Created on: Dec 21, 2025
 *      Author: 12114
 */

#ifndef HAL_RTC_HAL_RTC_HAL_H_
#define HAL_RTC_HAL_RTC_HAL_H_

#include "rtc_driver.h"
#include <stdint.h>

// 内存分配源
#define RTC_HAL_MEMSOURCE SYS_MEM_INTERNAL

// 前向声明
typedef struct rtc_hal_t rtc_hal_t;

// 单例模式接口
rtc_hal_t *rtc_hal_get_instance(void);
int rtc_hal_init(rtc_driver_t *driver, rtc_date_t *init_date,
                 rtc_time_t *init_time);

// RTC HAL 接口
int rtc_hal_set_time(rtc_time_t *time);
int rtc_hal_get_time(rtc_time_t *time);
int rtc_hal_set_date(rtc_date_t *date);
int rtc_hal_get_date(rtc_date_t *date);

// 获取 Unix 毫秒时间戳
uint64_t rtc_hal_get_unix_ms(void);

#endif /* HAL_RTC_HAL_RTC_HAL_H_ */
