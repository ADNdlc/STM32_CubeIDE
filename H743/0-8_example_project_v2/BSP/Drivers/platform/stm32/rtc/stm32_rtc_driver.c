/*
 * stm32_rtc_driver.c
 *
 *  Created on: Feb 13, 2026
 *      Author: Antigravity
 */

#include "stm32_rtc_driver.h"
#include <stdlib.h>
#include "MemPool.h"
#include <string.h>

#if (TARGET_PLATFORM == PLATFORM_STM32)

// 使用备份寄存器 0 存储初始化标志
#define RTC_BKP_INIT_FLAG_REG RTC_BKP_DR0
#define RTC_BKP_INIT_FLAG_VALUE 0x5051 // "SQ" or any magic number

typedef struct {
  rtc_driver_t base;
  RTC_HandleTypeDef *hrtc;
} stm32_rtc_driver_t;

static int stm32_rtc_set_time(rtc_driver_t *self, rtc_time_t *time) {
  stm32_rtc_driver_t *drv = (stm32_rtc_driver_t *)self;
  RTC_TimeTypeDef sTime = {0};

  sTime.Hours = time->hour;
  sTime.Minutes = time->minute;
  sTime.Seconds = time->second;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;

  if (HAL_RTC_SetTime(drv->hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
    return -1;
  }
  return 0;
}

static int stm32_rtc_get_time(rtc_driver_t *self, rtc_time_t *time) {
  stm32_rtc_driver_t *drv = (stm32_rtc_driver_t *)self;
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {
      0}; // Must read date after time to unlock shadow registers

  if (HAL_RTC_GetTime(drv->hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
    return -1;
  }
  HAL_RTC_GetDate(drv->hrtc, &sDate, RTC_FORMAT_BIN);

  time->hour = sTime.Hours;
  time->minute = sTime.Minutes;
  time->second = sTime.Seconds;

  return 0;
}

static int stm32_rtc_set_date(rtc_driver_t *self, rtc_date_t *date) {
  stm32_rtc_driver_t *drv = (stm32_rtc_driver_t *)self;
  RTC_DateTypeDef sDate = {0};

  sDate.Year = date->year;
  sDate.Month = date->month;
  sDate.Date = date->day;
  sDate.WeekDay = date->week;

  if (HAL_RTC_SetDate(drv->hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
    return -1;
  }
  return 0;
}

static int stm32_rtc_get_date(rtc_driver_t *self, rtc_date_t *date) {
  stm32_rtc_driver_t *drv = (stm32_rtc_driver_t *)self;
  RTC_DateTypeDef sDate = {0};

  if (HAL_RTC_GetDate(drv->hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
    return -1;
  }

  date->year = sDate.Year;
  date->month = sDate.Month;
  date->day = sDate.Date;
  date->week = sDate.WeekDay;

  return 0;
}

static bool stm32_rtc_set_initflag(rtc_driver_t *self) {
  stm32_rtc_driver_t *drv = (stm32_rtc_driver_t *)self;
  HAL_RTCEx_BKUPWrite(drv->hrtc, RTC_BKP_INIT_FLAG_REG,
                      RTC_BKP_INIT_FLAG_VALUE);
  return true;
}

static bool stm32_rtc_get_initflag(rtc_driver_t *self) {
  stm32_rtc_driver_t *drv = (stm32_rtc_driver_t *)self;
  return (HAL_RTCEx_BKUPRead(drv->hrtc, RTC_BKP_INIT_FLAG_REG) ==
          RTC_BKP_INIT_FLAG_VALUE);
}

static const rtc_driver_ops_t stm32_rtc_ops = {
    .set_time = stm32_rtc_set_time,
    .get_time = stm32_rtc_get_time,
    .set_date = stm32_rtc_set_date,
    .get_date = stm32_rtc_get_date,
    .set_initflag = stm32_rtc_set_initflag,
    .get_initflag = stm32_rtc_get_initflag,
};

rtc_driver_t *stm32_rtc_driver_create(RTC_HandleTypeDef *hrtc) {
  if (hrtc == NULL)
    return NULL;

#ifdef USE_MEMPOOL
  stm32_rtc_driver_t *drv = (stm32_rtc_driver_t *)sys_malloc(
      SYS_MEM_INTERNAL, sizeof(stm32_rtc_driver_t));
#else
  stm32_rtc_driver_t *drv = (stm32_rtc_driver_t *)malloc(sizeof(stm32_rtc_driver_t));
#endif
  if (drv) {
    memset(drv, 0, sizeof(stm32_rtc_driver_t));
    drv->base.ops = &stm32_rtc_ops;
    drv->hrtc = hrtc;
  }
  return (rtc_driver_t *)drv;
}

#endif