/*
 * humiture_test.c
 *
 *  Created on: Feb 10, 2026
 *      Author: Antigravity
 */
#include "test_config.h"
#if ENABLE_TEST_HUMITURE
#define LOG_TAG "TEST_HUM"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "Sys.h"
#include "elog.h"
#include "humiture_factory.h"
#include "test_framework.h"

static void humiture_test_setup(void) { log_i("Humiture test started."); }

static void humiture_test_loop(void) {
  static uint32_t last_tick = 0;
  if (sys_get_systick_ms() - last_tick < 2000)
    return;
  last_tick = sys_get_systick_ms();

  humiture_driver_t *sensor = humiture_driver_get(TH_SENSOR_ID_AMBIENT);
  if (sensor == NULL) {
    log_e("Failed to get sensor!");
    return;
  }

  static bool initialized = false;
  if (!initialized) {
    if (HUMITURE_INIT(sensor) == 0)
      initialized = true;
    else
      return;
  }

  float temp, humi;
  if (HUMITURE_READ_FLOAT(sensor, &temp, &humi) == 0) {
    log_i("Temp: %.1f C, Humi: %.1f %%", temp, humi);
  } else {
    log_w("Read failed!");
  }
}

REGISTER_TEST(humiture, "DHT11/DHT22 sensor test", humiture_test_setup,
              humiture_test_loop, NULL);
#endif
