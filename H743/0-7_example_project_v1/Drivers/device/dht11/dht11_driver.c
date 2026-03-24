/*
 * dht11_driver.c
 *
 *  Created on: Jan 16, 2026
 *      Author: Antigravity
 */

#define LOG_TAG "DHT11"

#include "dht11_driver.h"
#include "SYSTEM/sys.h"
#include "elog.h"

// 内部宏定义
#define DHT11_RST_TIME_MS 20
#define DHT11_WAIT_TIME_US 30

static int dht11_check(dht11_driver_t *self) {
  uint8_t retry = 0;
  ONE_WIRE_SET_MODE(self->ow_drv, 0); // 输入模式

  // 等待拉低
  while (ONE_WIRE_READ(self->ow_drv) && retry < 200) {
    retry++;
    sys_delay_us(1);
  }
  if (retry >= 200) {
    log_i("dht11_check: timeout waiting for low (retry=%d, val=%d)", retry,
          ONE_WIRE_READ(self->ow_drv));
    return -1;
  }

  retry = 0;
  // 等待拉高
  while (!ONE_WIRE_READ(self->ow_drv) && retry < 200) {
    retry++;
    sys_delay_us(1);
  }
  if (retry >= 200) {
    log_i("dht11_check: timeout waiting for high (retry=%d, val=%d)", retry,
          ONE_WIRE_READ(self->ow_drv));
    return -2;
  }

  return 0;
}

static uint8_t dht11_read_bit(dht11_driver_t *self) {
  uint8_t retry = 0;
  // 等待变低
  while (ONE_WIRE_READ(self->ow_drv) && retry < 100) {
    retry++;
    sys_delay_us(1);
  }
  retry = 0;
  // 等待变高
  while (!ONE_WIRE_READ(self->ow_drv) && retry < 100) {
    retry++;
    sys_delay_us(1);
  }
  sys_delay_us(40); // 延时判断 40us
  if (ONE_WIRE_READ(self->ow_drv))
    return 1;
  else
    return 0;
}

static uint8_t dht11_read_byte(dht11_driver_t *self) {
  uint8_t i, dat = 0;
  for (i = 0; i < 8; i++) {
    dat <<= 1;
    dat |= dht11_read_bit(self);
  }
  return dat;
}

int dht11_init(dht11_driver_t *self, one_wire_driver_t *ow_drv) {
  if (!self || !ow_drv)
    return -1;
  self->ow_drv = ow_drv;
  self->humidity = 0;
  self->temperature = 0;

  ONE_WIRE_INIT(self->ow_drv);

  log_i("dht11_init: starting reset sequence");
  // 发送起始信号测试
  ONE_WIRE_SET_MODE(self->ow_drv, 1);
  ONE_WIRE_WRITE(self->ow_drv, 0);
  sys_delay_ms(DHT11_RST_TIME_MS);
  ONE_WIRE_WRITE(self->ow_drv, 1);
  sys_delay_us(DHT11_WAIT_TIME_US);
  log_i("dht11_init: reset pulse sent, waiting for response");

  if (dht11_check(self) != 0) {
    log_e("DHT11 initialization check failed!");
    return -1;
  }

  log_i("DHT11 initialized successfully.");
  return 0;
}

int dht11_read_data(dht11_driver_t *self) {
  uint8_t buf[5];
  uint8_t i;

  ONE_WIRE_SET_MODE(self->ow_drv, 1);
  ONE_WIRE_WRITE(self->ow_drv, 0);
  sys_delay_ms(DHT11_RST_TIME_MS);
  ONE_WIRE_WRITE(self->ow_drv, 1);
  sys_delay_us(DHT11_WAIT_TIME_US);

  if (dht11_check(self) == 0) {
    for (i = 0; i < 5; i++) {
      buf[i] = dht11_read_byte(self);
    }
    if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4]) {
      self->humidity = buf[0];
      self->temperature = buf[2];
      return 0;
    } else {
      log_w("DHT11 checksum error!");
      return -2;
    }
  } else {
    log_w("DHT11 no response!");
    return -1;
  }
}
