/*
 * bh1750_driver.c
 *
 *  Created on: Jan 25, 2026
 *      Author: Antigravity
 */

#define LOG_TAG "BH1750"

#include "bh1750_driver.h"
#include "elog.h"
#include "sys.h"
#include <string.h>


/**
 * @brief 向 BH1750 发送单字节指令
 */
static int bh1750_write_cmd(bh1750_driver_t *self, uint8_t cmd) {
  // 对于单字节指令，通常直接通过 I2C 发送即可
  // 这里使用 I2C_START/SEND/STOP 的组合，或者如果有封装好的 i2c_write
  // 由于接口层目前主要提供了 write_reg，通常 write_reg
  // 也可以用来发指令（reg=cmd, len=0） 但具体取决于 i2c_driver 的实现。软件 I2C
  // 实现通常支持这种方式。

  I2C_START(self->config.i2c);
  I2C_SEND_BYTE(self->config.i2c, (self->config.i2c_addr << 1) | 0);
  if (I2C_WAIT_ACK(self->config.i2c) != 0) {
    I2C_STOP(self->config.i2c);
    return -1;
  }
  I2C_SEND_BYTE(self->config.i2c, cmd);
  I2C_WAIT_ACK(self->config.i2c);
  I2C_STOP(self->config.i2c);

  return 0;
}

int bh1750_init(bh1750_driver_t *self, const bh1750_config_t *config) {
  if (!self || !config || !config->i2c)
    return -1;

  memset(self, 0, sizeof(bh1750_driver_t));
  self->config = *config;

  // 初始化底层 I2C
  I2C_INIT(self->config.i2c);

  // 1. 上电
  if (bh1750_write_cmd(self, BH1750_CMD_POWER_ON) != 0) {
    log_e("BH1750 power on failed!");
    return -2;
  }

  // 2. 复位
  bh1750_write_cmd(self, BH1750_CMD_RESET);

  // 3. 设置为连续高分辨率模式
  if (bh1750_write_cmd(self, BH1750_CMD_CH_RES) != 0) {
    log_e("BH1750 set mode failed!");
    return -3;
  }

  sys_delay_ms(180); // 等待第一次测量完成 (max 180ms)

  self->initialized = 1;
  log_i("BH1750 initialized successfully.");
  return 0;
}

int bh1750_read_lux(bh1750_driver_t *self) {
  if (!self || !self->initialized)
    return -1;

  uint8_t buf[2];

  // 读取 2 字节数据
  I2C_START(self->config.i2c);
  I2C_SEND_BYTE(self->config.i2c, (self->config.i2c_addr << 1) | 1); // 读操作
  if (I2C_WAIT_ACK(self->config.i2c) != 0) {
    I2C_STOP(self->config.i2c);
    return -2;
  }

  buf[0] = I2C_READ_BYTE(self->config.i2c, 1); // ACK
  buf[1] = I2C_READ_BYTE(self->config.i2c, 0); // NACK
  I2C_STOP(self->config.i2c);

  uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1];
  self->lux = (float)raw / 1.2f;

  return 0;
}
