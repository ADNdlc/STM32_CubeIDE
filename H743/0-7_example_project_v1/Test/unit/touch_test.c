/*
 * touch_test.c
 *
 *  Created on: Dec 13, 2025
 *      Author: Antigravity
 *
 *  触摸屏驱动测试
 *  演示如何使用重构后的触摸屏驱动架构
 */

 #include "all_tests_config.h"
#if _touch_test_

#include "touch_test.h"
#include "sys.h"
#include "touch_driver.h"
#include "touch_factory.h"
#include "elog.h"
#include <stdio.h>


#define LOG_TAG "TOUCH_TEST"

/**
 * @brief 初始化测试
 */
static int test_touch_init(touch_driver_t *touch) {
  log_i("[%s] Testing touch init...", LOG_TAG);

  int ret = TOUCH_INIT(touch);
  if (ret != 0) {
    log_e("[%s] Touch init failed: %d", LOG_TAG, ret);
    return -1;
  }

  const char *device_id = TOUCH_GET_DEVICE_ID(touch);
  uint8_t max_points = TOUCH_GET_MAX_POINTS(touch);

  log_i("[%s] Touch init success!", LOG_TAG);
  log_i("[%s] Device ID: %s", LOG_TAG, device_id);
  log_i("[%s] Max touch points: %d", LOG_TAG, max_points);

  return 0;
}

/**
 * @brief 扫描测试
 */
static int test_touch_scan(touch_driver_t *touch) {
  log_i("[%s] Testing touch scan (10s)...", LOG_TAG);
  log_i("[%s] Please touch the screen!", LOG_TAG);

  touch_data_t data;
  uint32_t start_time = sys_get_systick_ms();
  uint32_t touch_count = 0;

  // 测试 10 秒
  while (sys_get_systick_ms() - start_time < 10000) {
    int ret = TOUCH_SCAN(touch, &data);

    if (ret > 0) {
      touch_count++;
      log_i("[%s] Touch detected (%lu): %d points", LOG_TAG, touch_count,
             data.count);
      for (uint8_t i = 0; i < data.count; i++) {
        log_i("  Point %d: (%d, %d)", i, data.points[i].x, data.points[i].y);
      }
    } else if (ret < 0) {
      log_e("[%s] Touch scan error: %d", LOG_TAG, ret);
      return -1;
    }

    sys_delay_ms(50); // 20Hz 扫描频率
  }

  log_i("[%s] Touch scan test complete, total touches: %lu", LOG_TAG,
         touch_count);
  return 0;
}

/**
 * @brief 运行触摸屏测试
 */
int touch_test_run(void) {
  log_i("[%s] Touch Driver Test Start", LOG_TAG);

  // 获取触摸屏驱动实例
  touch_driver_t *touch = touch_driver_get(TOUCH_MAIN);
  if (touch == NULL) {
    log_e("[%s] Failed to get touch driver!", LOG_TAG);
    return -1;
  }
  log_i("[%s] Touch driver instance acquired", LOG_TAG);

  // 测试初始化
  if (test_touch_init(touch) != 0) {
    return -1;
  }

  // 测试扫描
  if (test_touch_scan(touch) != 0) {
    return -1;
  }

  log_i("[%s] All Touch Tests Passed!", LOG_TAG);

  return 0;
}

#endif