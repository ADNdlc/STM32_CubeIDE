/*
 * bh1750_test.c
 *
 *  Created on: Jan 25, 2026
 *      Author: Antigravity
 */

#define LOG_TAG "BH1750_TEST"

#include "bh1750/bh1750_driver.h"
#include "elog.h"
#include "factory/i2c_factory.h"
#include "sys.h"


void bh1750_test_run(void) {
  static bh1750_driver_t bh1750;
  i2c_driver_t *i2c_drv;
  int res;

  log_i("Starting BH1750 Test...");

  // 从映射表中查找配置 (LS_BH1750)
  // 注意：实际项目中可能需要一个 bh1750_factory，或者仿照 dht11 直接从 mapping
  // 获取 这里直接使用映射表中的 I2C 设备和地址进行初始化
  extern const bh1750_mapping_t bh1750_mappings[];

  // 获取 I2C 驱动实例
  i2c_drv = i2c_soft_driver_get(bh1750_mappings[LS_BH1750].i2c_id);
  if (!i2c_drv) {
    log_e("Failed to get I2C driver instance for BH1750!");
    return;
  }

  // 组装驱动配置
  bh1750_config_t config = {
      .i2c = i2c_drv,
      .i2c_addr = bh1750_mappings[LS_BH1750].i2c_addr,
  };

  // 初始化 BH1750
  res = bh1750_init(&bh1750, &config);
  if (res != 0) {
    log_e("BH1750 initialization failed with code: %d", res);
    return;
  }

  log_i("BH1750 Test Loop Start (every 1 second)");

  while (1) {
    res = bh1750_read_lux(&bh1750);
    if (res == 0) {
      log_i("BH1750 Light Intensity: %.2f Lux", bh1750.lux);
    } else {
      log_w("BH1750 Read failed, error code: %d", res);
    }

    sys_delay_ms(1000);
  }
}
