#include "all_tests_config.h"
#if _rtc_test_
#include "elog.h"
#include "rtc_factory.h"
#include "rtc_hal/rtc_hal.h"
#include "rtc_test.h"
#include "sys.h"

void rtc_test_run(void) {
  log_i("RTC Test Start...");

  // 1. 获取驱动实例
  rtc_driver_t *driver = rtc_driver_get(RTC_DEVICE_0);
  if (!driver) {
    log_e("Failed to get RTC driver!");
    return;
  }

  // 准备初始化日期和时间 (由业务端确定)
  rtc_date_t init_date = {.year = 25, .month = 12, .day = 21, .week = 7};
  rtc_time_t init_time = {.hour = 19, .minute = 45, .second = 0};

  // 2. 初始化 HAL (单例)
  int init_status = rtc_hal_init(driver, &init_date, &init_time);
  if (init_status == 1) {
    log_i("RTC HAL newly initialized with custom data.");
  } else if (init_status == 0) {
    log_i("RTC HAL already initialized, skipping custom data.");
  } else {
    log_e("RTC HAL initialization failed!");
    return;
  }

  // 3. 循环获取当前时间并打印
  rtc_time_t current_time;
  rtc_date_t current_date;

  log_i("Starting cyclical time printing (Press any key or wait for timeout if "
        "implemented)...");

  for (int i = 0; i < 10; i++) {
    if (rtc_hal_get_time(&current_time) == 0 &&
        rtc_hal_get_date(&current_date) == 0) {
      log_i("[%d] RTC: 20%02d-%02d-%02d Week:%d %02d:%02d:%02d", i + 1,
            current_date.year, current_date.month, current_date.day,
            current_date.week, current_time.hour, current_time.minute,
            current_time.second);
    } else {
      log_e("Failed to get RTC time/date!");
    }
    sys_delay_ms(1000); // 延时 1 秒
  }

  log_i("RTC Test End.");
}

#endif
