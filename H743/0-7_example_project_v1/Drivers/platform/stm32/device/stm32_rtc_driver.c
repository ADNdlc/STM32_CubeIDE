/*
 * stm32_rtc_driver.c
 *
 *  Created on: Dec 21, 2025
 *      Author: 12114
 */

#include "stm32_rtc_driver.h"
#include "stm32h7xx_hal.h"
#include <stdlib.h>
#include <string.h>

// 定义RTC备份寄存器用于存储初始化标志
#define RTC_INIT_FLAG_REG 0U       // 标志位使用的备份寄存器
#define RTC_INIT_FLAG_VALUE 0xA4D7 // 标志位的值

// 函数声明
static int _stm32_rtc_set_time(rtc_driver_t *base, rtc_time_t *time);
static int _stm32_rtc_get_time(rtc_driver_t *base, rtc_time_t *time);
static int _stm32_rtc_set_date(rtc_driver_t *base, rtc_date_t *date);
static int _stm32_rtc_get_date(rtc_driver_t *base, rtc_date_t *date);
static bool _stm32_rtc_set_initflag(rtc_driver_t *base);
static bool _stm32_rtc_get_initflag(rtc_driver_t *base);

// Vtable definition
static const rtc_driver_ops_t _stm32_rtc_driver_ops = {
    .set_time = _stm32_rtc_set_time,
    .get_time = _stm32_rtc_get_time,
    .set_date = _stm32_rtc_set_date,
    .get_date = _stm32_rtc_get_date,
    .set_initflag = _stm32_rtc_set_initflag,
    .get_initflag = _stm32_rtc_get_initflag,
};

/* ==========================================
 * 创建STM32 RTC驱动实例
 * ========================================== */
rtc_driver_t *stm32_rtc_driver_create(RTC_HandleTypeDef *hrtc) {
  stm32_rtc_driver_t *driver =
      (stm32_rtc_driver_t *)malloc(sizeof(stm32_rtc_driver_t));
  if (driver) {
    driver->base.ops = &_stm32_rtc_driver_ops; // 绑定平台相关操作接口
    driver->hrtc = hrtc;
  }
  return (rtc_driver_t *)driver;
}

/* ==========================================
 * STM32 RTC驱动接口实现
 * ========================================== */
static int _stm32_rtc_set_time(rtc_driver_t *base, rtc_time_t *time) {
  stm32_rtc_driver_t *self = (stm32_rtc_driver_t *)base;

  RTC_TimeTypeDef sTime = {0};
  sTime.Hours = time->hour;
  sTime.Minutes = time->minute;
  sTime.Seconds = time->second;

  if (HAL_RTC_SetTime(self->hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
    return -1; // 错误
  }

  return 0; // 成功
}

static int _stm32_rtc_get_time(rtc_driver_t *base, rtc_time_t *time) {
  stm32_rtc_driver_t *self = (stm32_rtc_driver_t *)base;

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0}; // 必须读取日期才能解锁影子寄存器

  if (HAL_RTC_GetTime(self->hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
    return -1; // 错误
  }

  // STM32 影子寄存器机制：读取时间后必须读取日期，否则影子寄存器将锁定
  HAL_RTC_GetDate(self->hrtc, &sDate, RTC_FORMAT_BIN);

  time->hour = sTime.Hours;
  time->minute = sTime.Minutes;
  time->second = sTime.Seconds;

  return 0; // 成功
}

static int _stm32_rtc_set_date(rtc_driver_t *base, rtc_date_t *date) {
  stm32_rtc_driver_t *self = (stm32_rtc_driver_t *)base;

  RTC_DateTypeDef sDate = {0};
  sDate.Year = date->year;
  sDate.Month = date->month;
  sDate.Date = date->day;
  sDate.WeekDay = date->week;

  if (HAL_RTC_SetDate(self->hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
    return -1; // 错误
  }

  return 0; // 成功
}

static int _stm32_rtc_get_date(rtc_driver_t *base, rtc_date_t *date) {
  stm32_rtc_driver_t *self = (stm32_rtc_driver_t *)base;

  RTC_DateTypeDef sDate = {0};

  if (HAL_RTC_GetDate(self->hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
    return -1; // 错误
  }

  date->year = sDate.Year;
  date->month = sDate.Month;
  date->day = sDate.Date;
  date->week = sDate.WeekDay;

  return 0; // 成功
}

static bool _stm32_rtc_set_initflag(rtc_driver_t *base) {
  stm32_rtc_driver_t *self = (stm32_rtc_driver_t *)base;
  // 使用RTC备份寄存器来存储初始化标志
  HAL_RTCEx_BKUPWrite(self->hrtc, RTC_INIT_FLAG_REG, RTC_INIT_FLAG_VALUE);
  return true;
}

static bool _stm32_rtc_get_initflag(rtc_driver_t *base) {
  stm32_rtc_driver_t *self = (stm32_rtc_driver_t *)base;
  // 从RTC备份寄存器读取初始化标志并进行判断
  uint32_t value = HAL_RTCEx_BKUPRead(self->hrtc, RTC_INIT_FLAG_REG);
  return (value == RTC_INIT_FLAG_VALUE);
}
