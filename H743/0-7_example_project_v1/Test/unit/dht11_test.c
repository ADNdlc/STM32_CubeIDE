/*
 * dht11_test.c
 *
 *  Created on: Jan 16, 2026
 *      Author: Antigravity
 */

#define LOG_TAG "DHT11_TEST"

#include "SYSTEM/sys.h"
#include "device/dht11/dht11_driver.h"
#include "elog.h"
#include "factory/one_wire_factory.h"

void dht11_test_run(void) {
  static dht11_driver_t dht11;
  one_wire_driver_t *ow_drv;
  int res;

  log_i("Starting DHT11 Test...");

  // 获取 One-Wire 驱动实例 (PB11)
  ow_drv = one_wire_soft_driver_get(ONE_WIRE_SOFT_DHT11);
  if (!ow_drv) {
    log_e("Failed to get One-Wire driver instance!");
    return;
  }

  // 初始化 DHT11
  res = dht11_init(&dht11, ow_drv);
  if (res != 0) {
    log_e("DHT11 initialization failed with code: %d", res);
    // Continue anyway to see if it responds later? No, usually init must pass.
  }

  log_i("DHT11 Test Loop Start (every 2 seconds)");

  while (1) {
    res = dht11_read_data(&dht11);
    if (res == 0) {
      log_i("DHT11 Data: Humidity = %d %%, Temperature = %d C", dht11.humidity,
            dht11.temperature);
    } else {
      log_w("DHT11 Read failed, error code: %d", res);

      // If read failed, we might want to try re-initializing if it was a setup
      // issue
      if (res == -1) {
        // Optionally retry init if no response
      }
    }

    sys_delay_ms(2000);
  }
}
