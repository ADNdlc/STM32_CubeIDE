/*
 * all_tests.c
 *
 *  Created on: Dec 3, 2025
 *      Author: 12114
 */
#include "project_cfg.h"

#if TEST_ENABLE
#include "all_tests_config.h"
#include "elog.h"
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
#include "norflash_test.h"
#include "lfs_test.h"
#include "sys_config_test.h"  // 添加sys_config测试头文件
#include "connectivity_test.h"

#define TEST_MODULE(func)  do{		\
	log_w(#func" is running...");	\
	func;	\
}while(0)

void run_all_tests(void) {
	log_w(">>>>> Run Test !!! <<<<<");

#if _led_test_
  TEST_MODULE(led_test_run());
#endif

#if _key_test_
  TEST_MODULE(key_test_run());
#endif

#if _uart_hal_test_
  TEST_MODULE(uart_hal_test_run());
#endif

#if _uart_queue_test_
  TEST_MODULE(uart_queue_test_run());
#endif

#if _elog_test_
  TEST_MODULE(elog_test_run());
#endif

#if _sdram_test_
  TEST_MODULE(sdram_test());
#endif

#if _sys_test_
  TEST_MODULE(sys_test());
#endif

#if _lcd_test_
  TEST_MODULE(lcd_test_run());
#endif

#if _lvgl_test_
  TEST_MODULE(lvgl_test_run());
#endif

#if _touch_test_
  TEST_MODULE(touch_test_run());
#endif

#if _rtc_test_
  TEST_MODULE(rtc_test_run());
#endif

#if _wifi_test_
  TEST_MODULE(wifi_test_run());
#endif

#if _norflash_test_
  TEST_MODULE(norflash_test());
#endif

#if _flash_test_
  TEST_MODULE(flash_integration_test());
#endif

#if _lfs_test_
  TEST_MODULE(lfs_integration_test());
#endif

#if _sys_config_test_
  TEST_MODULE(sys_config_test_run());
#endif

#if _net_mgr_test_
  TEST_MODULE(connectivity_test_run());
#endif
}
#else
void run_all_tests(void) {}
#endif
