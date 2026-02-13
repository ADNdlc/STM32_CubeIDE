/*
 * touch_test.c
 *
 *  Created on: Feb 12, 2026
 *      Author: Antigravity
 */
#include "test_config.h"
#if ENABLE_TEST_TOUCH

#define LOG_TAG "TEST_TOUCH"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "Sys.h"
#include "elog.h"
#include "touch_factory.h"
#include "test_framework.h"

  touch_driver_t *drv;

static void touch_test_setup(void) {
  log_i("Touch test started.");
  drv = touch_driver_get(TOUCH_ID_UI);
  if (drv) {
    log_i("Touch driver instance found.");
  } else {
    log_e("Failed to get touch driver!");
  }
}

static void touch_test_loop(void) {
  static uint32_t last_tick = 0;
  if (sys_get_systick_ms() - last_tick < 20) // 50Hz 采样
    return;
  last_tick = sys_get_systick_ms();
  if (drv == NULL)
    return;

  if (TOUCH_SCAN(drv) > 0) {
    Touch_Data_t *data = TOUCH_GET_DATA(drv);
    log_i("Touch Detected: %d points", data->point_count);
    for (int i = 0; i < data->point_count; i++) {
      log_i("  P%d: [%d, %d]", i, data->points[i].x, data->points[i].y);
    }
  }
}

static void touch_test_teardown(){
  drv = NULL;
  log_i("Touch test teardown.");
}

REGISTER_TEST(touch, "GT9xxx touch screen test", touch_test_setup,
              touch_test_loop, touch_test_teardown);
#endif

