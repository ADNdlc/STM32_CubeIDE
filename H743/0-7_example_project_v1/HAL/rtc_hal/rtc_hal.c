#include "rtc_hal.h"
#include "rtc_driver.h"
#include "sys.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


// 定义rtc_hal类结构体
struct rtc_hal_t {
  rtc_driver_t *driver; // 底层驱动实例
};

// 单例实例
static rtc_hal_t *s_rtc_hal_instance = NULL;

/* ==========================================
 * 单例模式实现
 * ========================================== */
rtc_hal_t *rtc_hal_get_instance(void) {
  if (s_rtc_hal_instance == NULL) {
    // 创建单例实例
    s_rtc_hal_instance =
        (rtc_hal_t *)sys_malloc(RTC_HAL_MEMSOURCE, sizeof(rtc_hal_t));
    if (s_rtc_hal_instance) {
      s_rtc_hal_instance->driver = NULL;
    }
  }
  return s_rtc_hal_instance; // 返回单例实例
}

int rtc_hal_init(rtc_driver_t *driver, rtc_date_t *init_date,
                 rtc_time_t *init_time) {
  rtc_hal_t *self = rtc_hal_get_instance();
  if (!self || !driver)
    return -1;

  self->driver = driver;

  // 判断是否需要重新设置时间
  if (!RTC_GET_INITFLAG(self->driver)) {
    // 如果业务端提供了初始化数据，则使用业务端的数据
    if (init_date && init_time) {
      RTC_SET_DATE(self->driver, init_date);
      RTC_SET_TIME(self->driver, init_time);
    } else {
      // 设置默认日期: 2025-12-21
      rtc_date_t default_date = {
          .year = 25,
          .month = 12,
          .day = 21,
          .week = 7 // Sunday
      };
      // 设置默认时间: 19:00:00
      rtc_time_t default_time = {.hour = 19, .minute = 0, .second = 0};

      RTC_SET_DATE(self->driver, &default_date);
      RTC_SET_TIME(self->driver, &default_time);
    }

    // 设置初始化标志
    RTC_SET_INITFLAG(self->driver);
    return 1; // 新初始化
  }
  return 0; // 已初始化
}

/* ==========================================
 * RTC HAL接口实现
 * ========================================== */
int rtc_hal_set_time(rtc_time_t *time) {
  rtc_hal_t *self = rtc_hal_get_instance();
  if (!self || !self->driver)
    return -1;
  return RTC_SET_TIME(self->driver, time);
}

int rtc_hal_get_time(rtc_time_t *time) {
  rtc_hal_t *self = rtc_hal_get_instance();
  if (!self || !self->driver)
    return -1;
  return RTC_GET_TIME(self->driver, time);
}

int rtc_hal_set_date(rtc_date_t *date) {
  rtc_hal_t *self = rtc_hal_get_instance();
  if (!self || !self->driver)
    return -1;
  return RTC_SET_DATE(self->driver, date);
}

int rtc_hal_get_date(rtc_date_t *date) {
  rtc_hal_t *self = rtc_hal_get_instance();
  if (!self || !self->driver)
    return -1;
  return RTC_GET_DATE(self->driver, date);
}

uint64_t rtc_hal_get_unix_ms(void) {
  rtc_time_t rt;
  rtc_date_t rd;
  if (rtc_hal_get_time(&rt) != 0 || rtc_hal_get_date(&rd) != 0)
    return 0;

  struct tm t;
  memset(&t, 0, sizeof(struct tm));
  t.tm_year = rd.year + 2000 - 1900;
  t.tm_mon = rd.month - 1;
  t.tm_mday = rd.day;
  t.tm_hour = rt.hour;
  t.tm_min = rt.minute;
  t.tm_sec = rt.second;

  time_t s = mktime(&t);
  if (s == (time_t)-1)
    return 0;

  return (uint64_t)s * 1000 + (sys_get_systick_ms() % 1000);
}
