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
illuminance_driver_t *sensor;

static void illuminance_test_setup(void) {
  sensor = illuminance_driver_get(LIGHT_SENSOR_ID_AMBIENT);
  if (sensor == NULL) {
    log_e("Failed to get light sensor!");
    return;
  }
  if (ILLUMINANCE_INIT(sensor) != 0) {
    log_e("Failed to initialize light sensor!");
    return;
  }
  log_i("Illuminance test started.");
}

static void illuminance_test_loop(void) {
  static uint32_t last_tick = 0;
  if (sys_get_systick_ms() - last_tick < 1000)
    return;
  last_tick = sys_get_systick_ms();

  float lux;
  if (ILLUMINANCE_READ_LUX(sensor, &lux) == 0) {
#ifndef PRINTF_FLOAT_ENABLED
    int integer_part = (int)lux;
    int decimal_part = (int)((lux - integer_part) * 10);
    log_i("Illuminance: %d.%d Lux", integer_part, decimal_part);
#else
    log_i("Illuminance: %.1f Lux", lux);
#endif
  } else {
    log_w("Read failed!");
  }
}

static void illuminance_test_teardown(void) { log_i("Illuminance test done."); }

REGISTER_TEST(illuminance, "BH1750 light sensor test", illuminance_test_setup,
              illuminance_test_loop, illuminance_test_teardown);
#endif
