/*
 * all_tests.c
 *
 *  Created on: Dec 3, 2025
 *      Author: 12114
 */

#include "all_tests_config.h"
#include "elog_test.h"
#include "key_test.h"
#include "lcd_test.h"
#include "led_test.h"
#include "lvgl_test.h"
#include "rtc_test.h"
#include "sdram_test.h"
#include "sys_test.h"
#include "touch_test.h"
#include "uart_hal_test.h"
#include "uart_queue_test.h"
#include "wifi_test.h"
#include "flash_test.h"

void run_all_tests(void) {
#if _led_test_
  led_test_run();
#endif

#if _key_test_
  key_test_run();
#endif

#if _uart_hal_test_
  uart_hal_test_run();
#endif

#if _uart_queue_test_
  uart_queue_test_run();
#endif

#if _elog_test_
  elog_test_run();
#endif

#if _sdram_test_
  sdram_test();
#endif

#if _sys_test_
  sys_test();
#endif

#if _lcd_test_
  lcd_test_run();
#endif

#if _lvgl_test_
  lvgl_test_run();
#endif

#if _touch_test_
  touch_test_run();
#endif

#if _rtc_test_
  rtc_test_run();
#endif

#if _wifi_test_
  wifi_test_run();
#endif

#if _flash_test_
  flash_integration_test();
#endif
}
