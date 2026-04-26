/*
 * bh1750_driver.c
 *
 *  Created on: Feb 11, 2026
 *      Author: Antigravity
 */

#define LOG_TAG "BH1750"

#include "bh1750_driver.h"
#include "Sys.h"
#include "elog.h"
#include <stdlib.h>
#include "MemPool.h"
#ifdef USE_MEMPOOL
#define BH1750_MEMSOURCE SYS_MEM_INTERNAL // 从哪里分配
#endif

// BH1750 指令
// BH1750 指令定义
#define BH1750_CMD_POWER_DOWN 0x00
#define BH1750_POWER_ON 0x01
#define BH1750_RESET 0x07
#define BH1750_CMD_CH_RES 0x10  // 连续 H 分辨率模式 (1lx, 120ms)
#define BH1750_CMD_CH_RES2 0x11 // 连续 H 分辨率模式2 (0.5lx, 120ms)
#define BH1750_CMD_CL_RES 0x13  // 连续 L 分辨率模式 (4lx, 16ms)
#define BH1750_CMD_ONE_TIME_H_RES 0x20 // 一次性 H 分辨率模式 (1lx, 120ms),测试后自动进入断电模式
#define BH1750_CMD_ONE_TIME_H_RES2 0x21// 一次性 H 分辨率模式2 (0.5lx, 120ms),测试后自动进入断电模式
#define BH1750_CMD_ONE_TIME_L_RES 0x23 // 一次性 L 分辨率模式 (4lx, 16ms),测试后自动进入断电模式

// 默认 I2C 地址 (ADDR 引脚接地)
#define BH1750_DEFAULT_ADDR (0x23 << 1)

#define BH1750_MEMSOURCE SYS_MEM_INTERNAL

/* --- 接口实现 --- */

static int stm32_bh1750_init(illuminance_driver_t *self) {
  bh1750_driver_t *drv = (bh1750_driver_t *)self;
  if (!drv || !drv->i2c_drv)
    return -1;

  uint8_t cmd = BH1750_POWER_ON;
  if (I2C_MASTER_TRANSMIT(drv->i2c_drv, drv->dev_addr, &cmd, 1, 100) != 0) {
    log_e("BH1750 Power On failed!");
    return -1;
  }

  cmd = BH1750_CMD_CH_RES;
  if (I2C_MASTER_TRANSMIT(drv->i2c_drv, drv->dev_addr, &cmd, 1, 100) != 0) {
    log_e("BH1750 Set Mode failed!");
    return -1;
  }

  log_d("BH1750 initialized.");
  return 0;
}

static int stm32_bh1750_read_lux(illuminance_driver_t *self, float *lux) {
  bh1750_driver_t *drv = (bh1750_driver_t *)self;
  if (!drv || !drv->i2c_drv || !lux)
    return -1;

  uint8_t buffer[2];
  if (I2C_MASTER_RECEIVE(drv->i2c_drv, drv->dev_addr, buffer, 2, 200) != 0) {
    log_w("BH1750 Read Lux failed!");
    return -1;
  }

  uint16_t raw = (buffer[0] << 8) | buffer[1];
  *lux = (float)raw / 1.2f; // 公式: Lux = Raw / 1.2

  return 0;
}

static const illuminance_driver_ops_t bh1750_ops = {
    .init = stm32_bh1750_init,
    .read_lux = stm32_bh1750_read_lux,
};

/* --- 驱动构造 --- */

illuminance_driver_t *bh1750_driver_create(i2c_driver_t *i2c_drv) {

#ifdef USE_MEMPOOL
  bh1750_driver_t *drv =
      (bh1750_driver_t *)sys_malloc(BH1750_MEMSOURCE, sizeof(bh1750_driver_t));
#else
  bh1750_driver_t *drv =
      (bh1750_driver_t *)malloc(sizeof(bh1750_driver_t));
#endif

  if (drv) {
    drv->base.ops = &bh1750_ops;
    drv->i2c_drv = i2c_drv;
    drv->dev_addr = BH1750_DEFAULT_ADDR;
  }
  return (illuminance_driver_t *)drv;
}
