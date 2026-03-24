/*
 * rtc_test.c
 *
 *  Created on: Feb 13, 2026
 *      Author: Antigravity
 *
 *  RTC 测试：显示时间，设置时间
 */
#include "test_config.h"
#if ENABLE_TEST_RTC
#define LOG_TAG "TEST_RTC"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "Sys.h"
#include "elog.h"
#include "rtc_factory.h"
#include "test_framework.h"

 rtc_driver_t *rtc;

static void rtc_test_setup(void) {
  rtc = rtc_driver_get(RTC_ID_INTERNAL);
  if (!rtc) {
    log_e("RTC driver not found!");
    return;
  }

  if (!RTC_GET_INITFLAG(rtc)) {
    log_w("RTC not initialized, setting default time...");
    rtc_time_t time = {12, 0, 0};
    rtc_date_t date = {26, 2, 13, 5}; // 2026-02-13 Friday
    RTC_SET_TIME(rtc, &time);
    RTC_SET_DATE(rtc, &date);
    RTC_SET_INITFLAG(rtc);
    log_i("Default time set.");
  } else {
    log_i("RTC already initialized.");
  }
}

static void rtc_test_loop(void) {
  static uint32_t last_tick = 0;
  if (sys_get_systick_ms() - last_tick < 1000)
    return;
  last_tick = sys_get_systick_ms();
  if (!rtc)
    return;
  rtc_time_t time;
  rtc_date_t date;
  RTC_GET_TIME(rtc, &time);
  RTC_GET_DATE(rtc, &date);

  log_i("Current Time: 20%02d-%02d-%02d %02d:%02d:%02d (Week %d)", date.year,
        date.month, date.day, time.hour, time.minute, time.second, date.week);
}

static void rtc_test_teardown(){
  rtc = NULL;
  log_i("RTC test teardown.");
}

REGISTER_TEST(rtc, "Internal RTC test", rtc_test_setup, rtc_test_loop, rtc_test_teardown);
#endif
