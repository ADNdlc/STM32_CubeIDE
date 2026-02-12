/*
 * illuminance_test.c
 *
 *  Created on: Feb 11, 2026
 *      Author: Antigravity
 */
#include "test_config.h"
#if ENABLE_TEST_ILLUMINANCE
#include "Sys.h"
#include "elog.h"
#include "illuminance_factory.h"
#include "test_framework.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define LOG_TAG "TEST_LUX"

static void illuminance_test_setup(void) { log_i("Illuminance test started."); }

static void illuminance_test_loop(void) {
  static uint32_t last_tick = 0;
  if (sys_get_systick_ms() - last_tick < 1000)
    return;
  last_tick = sys_get_systick_ms();

  illuminance_driver_t *sensor =
      illuminance_driver_get(LIGHT_SENSOR_ID_AMBIENT);
  if (sensor == NULL) {
    log_e("Failed to get light sensor!");
    return;
  }

  static bool initialized = false;
  if (!initialized) {
    if (ILLUMINANCE_INIT(sensor) == 0)
      initialized = true;
    else
      return;
  }

  float lux;
  if (ILLUMINANCE_READ_LUX(sensor, &lux) == 0) {
    log_i("Illuminance: %.1f Lux", lux);
  } else {
    log_w("Read failed!");
  }
}

static void illuminance_test_teardown(void) { log_i("Illuminance test done."); }

REGISTER_TEST(illuminance, "BH1750 light sensor test", illuminance_test_setup,
              illuminance_test_loop, illuminance_test_teardown);
#endif
