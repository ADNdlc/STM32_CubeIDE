/*
 * rtc_driver.h
 *
 *  Created on: Dec 21, 2025
 *      Author: 12114
 */

#ifndef INTERFACE_RTC_DRIVER_H_
#define INTERFACE_RTC_DRIVER_H_

#include "stdbool.h"
#include <stdint.h>

// 前向声明
typedef struct rtc_driver_t rtc_driver_t;

// 定义时间
typedef struct {
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} rtc_time_t;
// 定义日期
typedef struct {
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t week;
} rtc_date_t;

// RTC 驱动操作接口
typedef struct {
	int (*set_time)(rtc_driver_t *rtc, rtc_time_t *time);
	int (*get_time)(rtc_driver_t *rtc, rtc_time_t *time);
	int (*set_date)(rtc_driver_t *rtc, rtc_date_t *date);
	int (*get_date)(rtc_driver_t *rtc, rtc_date_t *date);
	bool (*set_initflag)(rtc_driver_t *rtc);
	bool (*get_initflag)(rtc_driver_t *rtc);
} rtc_driver_ops_t;

struct rtc_driver_t {
	const rtc_driver_ops_t *ops;
};

// 辅助宏
#define RTC_SET_TIME(_rtc, _time) (_rtc)->ops->set_time(_rtc, _time)
#define RTC_GET_TIME(_rtc, _time) (_rtc)->ops->get_time(_rtc, _time)
#define RTC_SET_DATE(_rtc, _date) (_rtc)->ops->set_date(_rtc, _date)
#define RTC_GET_DATE(_rtc, _date) (_rtc)->ops->get_date(_rtc, _date)
#define RTC_SET_INITFLAG(_rtc) (_rtc)->ops->set_initflag(_rtc)
#define RTC_GET_INITFLAG(_rtc) (_rtc)->ops->get_initflag(_rtc)

#endif /* INTERFACE_RTC_DRIVER_H_ */