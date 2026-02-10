/*
 * dht11_driver.c
 *
 *  Created on: Jan 16, 2026
 *      Author: Antigravity
 */

#define LOG_TAG "DHT11"

#include "dht11_driver.h"
#include "MemPool.h"
#include "Sys.h"
#include "elog.h"
#include <stdlib.h>


// 内部宏定义
#define DHT11_RST_TIME_MS 20
#define DHT11_WAIT_TIME_US 30

#define DHT11_MEMSOURCE SYS_MEM_INTERNAL

// 内部函数声明
static int dht11_check(dht11_driver_t *self);
static uint8_t dht11_read_bit(dht11_driver_t *self);
static uint8_t dht11_read_byte(dht11_driver_t *self);
static int dht11_read_raw_internal(dht11_driver_t *self);

/* --- 接口实现 --- */
static int stm32_dht11_init(humiture_driver_t *self) {
  dht11_driver_t *drv = (dht11_driver_t *)self;
  if (!drv || !drv->ow_drv)
    return -1;
  ONE_WIRE_INIT(drv->ow_drv);
  // 发送起始信号测试
  ONE_WIRE_SET_MODE(drv->ow_drv, 1);
  ONE_WIRE_WRITE(drv->ow_drv, 0);
  sys_delay_ms(DHT11_RST_TIME_MS);
  ONE_WIRE_WRITE(drv->ow_drv, 1);
  sys_delay_us(DHT11_WAIT_TIME_US);

  if (dht11_check(drv) != 0) {
    log_e("DHT11 response check failed!");
    return -1;
  }

  log_d("DHT11 probe success.");
  return 0;
}

static int stm32_dht11_read_raw(humiture_driver_t *self, int16_t *temp_x10,
                                uint16_t *humi_x10) {
  dht11_driver_t *drv = (dht11_driver_t *)self;
  int ret = dht11_read_raw_internal(drv);
  if (ret == 0) {
    if (temp_x10)
      *temp_x10 = drv->temperature * 10;
    if (humi_x10)
      *humi_x10 = drv->humidity * 10;
  }
  return ret;
}

static int stm32_dht11_read_float(humiture_driver_t *self, float *temp,
                                  float *humi) {
  dht11_driver_t *drv = (dht11_driver_t *)self;
  int ret = dht11_read_raw_internal(drv);
  if (ret == 0) {
    if (temp)
      *temp = (float)drv->temperature;
    if (humi)
      *humi = (float)drv->humidity;
  }
  return ret;
}

static const humiture_driver_ops_t dht11_ops = {
    .init = stm32_dht11_init,
    .read_raw = stm32_dht11_read_raw,
    .read_float = stm32_dht11_read_float,
};

/* --- 驱动构造 --- */
humiture_driver_t *dht11_driver_create(one_wire_driver_t *ow_drv) {
  dht11_driver_t *drv =
      (dht11_driver_t *)sys_malloc(DHT11_MEMSOURCE, sizeof(dht11_driver_t));
  if (drv) {
    drv->base.ops = &dht11_ops;
    drv->ow_drv = ow_drv;
    drv->humidity = 0;
    drv->temperature = 0;
  }
  return (humiture_driver_t *)drv;
}

/* --- DHT11 协议私有实现 --- */
static int dht11_check(dht11_driver_t *self) {
  uint8_t retry = 0;
  ONE_WIRE_SET_MODE(self->ow_drv, 0); // 输入模式
  while (ONE_WIRE_READ(self->ow_drv) && retry < 200) {
    retry++;
    sys_delay_us(1);
  }
  if (retry >= 200)
    return -1;
  retry = 0;
  while (!ONE_WIRE_READ(self->ow_drv) && retry < 200) {
    retry++;
    sys_delay_us(1);
  }
  if (retry >= 200)
    return -2;
  return 0;
}

static uint8_t dht11_read_bit(dht11_driver_t *self) {
  uint8_t retry = 0;
  while (ONE_WIRE_READ(self->ow_drv) && retry < 100) {
    retry++;
    sys_delay_us(1);
  }
  retry = 0;
  while (!ONE_WIRE_READ(self->ow_drv) && retry < 100) {
    retry++;
    sys_delay_us(1);
  }
  sys_delay_us(40);
  return ONE_WIRE_READ(self->ow_drv) ? 1 : 0;
}

static uint8_t dht11_read_byte(dht11_driver_t *self) {
  uint8_t i, dat = 0;
  for (i = 0; i < 8; i++) {
    dat <<= 1;
    dat |= dht11_read_bit(self);
  }
  return dat;
}

static int dht11_read_raw_internal(dht11_driver_t *self) {
  uint8_t buf[5];
  ONE_WIRE_SET_MODE(self->ow_drv, 1);
  ONE_WIRE_WRITE(self->ow_drv, 0);
  sys_delay_ms(DHT11_RST_TIME_MS);
  ONE_WIRE_WRITE(self->ow_drv, 1);
  sys_delay_us(DHT11_WAIT_TIME_US);
  // 读取数据
  if (dht11_check(self) == 0) {
    for (int i = 0; i < 5; i++) {
      buf[i] = dht11_read_byte(self);
    }
    if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4]) {
      self->humidity = buf[0];
      self->temperature = buf[2];
      return 0;
    }
    return -2; // 校验和错误
  }
  return -1; // 无响应
}
