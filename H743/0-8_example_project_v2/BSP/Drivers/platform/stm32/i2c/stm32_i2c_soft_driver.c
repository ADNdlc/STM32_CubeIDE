/*
 * stm32_i2c_soft_driver.c
 *
 *  Created on: Feb 11, 2026
 *      Author: 12114
 */
#include "MemPool.h"
#include "Sys.h"
#include "stm32_i2c_driver.h"
#include <stdlib.h>

// 默认延时（微秒）
#define DEFAULT_I2C_DELAY_US 2

//==============================================================================
// 私有辅助函数
//==============================================================================

#define CALL_SOFT_RES(drv) (drv)->config.resource.soft_config

/**
 * @brief I2C 延时
 */
static inline void i2c_delay(stm32_i2c_driver_t *drv) {
  sys_delay_us(CALL_SOFT_RES(drv)->delay_us);
}

/**
 * @brief 设置 SCL 电平
 */
static inline void i2c_scl_set(stm32_i2c_driver_t *drv, uint8_t value) {
  HAL_GPIO_WritePin(CALL_SOFT_RES(drv)->scl_port, CALL_SOFT_RES(drv)->scl_pin,
                    value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief 设置 SDA 电平
 */
static inline void i2c_sda_set(stm32_i2c_driver_t *drv, uint8_t value) {
  HAL_GPIO_WritePin(CALL_SOFT_RES(drv)->sda_port, CALL_SOFT_RES(drv)->sda_pin,
                    value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief 读取 SDA 电平
 */
static inline uint8_t i2c_sda_read(stm32_i2c_driver_t *drv) {
  return (uint8_t)HAL_GPIO_ReadPin(CALL_SOFT_RES(drv)->sda_port,
                                   CALL_SOFT_RES(drv)->sda_pin);
}

//==============================================================================
// I2C 驱动接口实现
//==============================================================================

/**
 * @brief 初始化 I2C GPIO
 */
static int stm32_i2c_soft_init(stm32_i2c_driver_t *self) {
  GPIO_InitTypeDef gpio_init;
  // 使能 GPIO 时钟（假设已在外部使能，这里只做引脚配置）
  // SCL 配置为开漏输出（用于I2C时钟线需要支持时钟拉伸）
  gpio_init.Pin = CALL_SOFT_RES(self)->scl_pin;
  gpio_init.Mode = GPIO_MODE_OUTPUT_OD;
  gpio_init.Pull = GPIO_PULLUP;
  gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(CALL_SOFT_RES(self)->scl_port, &gpio_init);

  // SDA 配置为开漏输出（用于双向数据传输）
  gpio_init.Pin = CALL_SOFT_RES(self)->sda_pin;
  gpio_init.Mode = GPIO_MODE_OUTPUT_OD;
  gpio_init.Pull = GPIO_PULLUP;
  gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(CALL_SOFT_RES(self)->sda_port, &gpio_init);

  // 初始状态：空闲（SCL=1, SDA=1）
  i2c_sda_set(self, 1);
  i2c_scl_set(self, 1);

  return 0;
}

/**
 * @brief 产生 I2C 起始信号
 * START: SCL 为高时，SDA 从高变低
 */
static void stm32_i2c_soft_start(stm32_i2c_driver_t *self) {
  i2c_sda_set(self, 1);
  i2c_scl_set(self, 1);
  i2c_delay(self);
  i2c_sda_set(self, 0); // START: SDA 从高变低
  i2c_delay(self);
  i2c_scl_set(self, 0); // 钳住总线，准备发送/接收数据
  i2c_delay(self);
}

/**
 * @brief 产生 I2C 停止信号
 * STOP: SCL 为高时，SDA 从低变高
 */
static void stm32_i2c_soft_stop(stm32_i2c_driver_t *self) {
  i2c_sda_set(self, 0); // 确保 SDA 为低
  i2c_delay(self);
  i2c_scl_set(self, 1); // SCL 变高
  i2c_delay(self);
  i2c_sda_set(self, 1); // STOP: SDA 从低变高
  i2c_delay(self);
}

/**
 * @brief 等待应答信号
 * @return 0=收到ACK, 1=超时或NACK
 */
static uint8_t stm32_i2c_soft_wait_ack(stm32_i2c_driver_t *self) {
  uint8_t wait_time = 0;
  uint8_t rack = 0;

  // 释放 SDA（主机释放，等待从机应答）
  i2c_sda_set(self, 1);
  i2c_delay(self);
  i2c_scl_set(self, 1); // SCL=1，此时从机可以返回 ACK
  i2c_delay(self);

  // 等待 SDA 变低（ACK）
  while (i2c_sda_read(self)) {
    wait_time++;
    if (wait_time > 250) {
      stm32_i2c_soft_stop(self);
      rack = 1;
      break;
    }
    i2c_delay(self);
  }

  i2c_scl_set(self, 0); // SCL=0，结束 ACK 检查
  i2c_delay(self);

  return rack;
}

/**
 * @brief 发送 ACK 应答
 */
static void stm32_i2c_soft_ack(stm32_i2c_driver_t *self) {
  // ACK: SCL 0->1 时 SDA=0
  i2c_sda_set(self, 0);
  i2c_delay(self);
  i2c_scl_set(self, 1);
  i2c_delay(self);
  i2c_scl_set(self, 0);
  i2c_delay(self);
  i2c_sda_set(self, 1); // 释放 SDA
  i2c_delay(self);
}

/**
 * @brief 发送 NACK 应答
 */
static void stm32_i2c_soft_nack(stm32_i2c_driver_t *self) {
  // NACK: SCL 0->1 时 SDA=1
  i2c_sda_set(self, 1);
  i2c_delay(self);
  i2c_scl_set(self, 1);
  i2c_delay(self);
  i2c_scl_set(self, 0);
  i2c_delay(self);
}

/**
 * @brief 发送一个字节
 * MSB 先发
 */
static void stm32_i2c_soft_send_byte(stm32_i2c_driver_t *self, uint8_t data) {
  for (uint8_t i = 0; i < 8; i++) {
    i2c_sda_set(self, (data & 0x80) >> 7); // 发送最高位
    i2c_delay(self);
    i2c_scl_set(self, 1); // SCL 上升沿锁存数据
    i2c_delay(self);
    i2c_scl_set(self, 0); // 准备下一位
    data <<= 1;           // 左移，准备发送下一位
  }
  // 释放 SDA
  i2c_sda_set(self, 1);
}

/**
 * @brief 读取一个字节
 * @param ack 1=发送ACK, 0=发送NACK
 */
static uint8_t stm32_i2c_soft_read_byte(stm32_i2c_driver_t *self, uint8_t ack) {
  uint8_t receive = 0;
  // 释放 SDA，准备读取
  i2c_sda_set(self, 1);

  for (uint8_t i = 0; i < 8; i++) {
    receive <<= 1;        // 左移（高位先接收）
    i2c_scl_set(self, 1); // SCL 上升沿读取数据
    i2c_delay(self);
    if (i2c_sda_read(self)) {
      receive++; // 读取到高电平
    }
    i2c_scl_set(self, 0);
    i2c_delay(self);
  }
  // 发送应答
  if (ack) {
    // 发送 ACK (SDA=0)
    stm32_i2c_soft_ack(self);
  } else {
    // 发送 NACK (SDA=1)
    stm32_i2c_soft_nack(self);
  }

  return receive;
}

static int stm32_soft_i2c_master_transmit(i2c_driver_t *self, uint16_t dev_addr,
                                          const uint8_t *data, uint16_t size,
                                          uint32_t timeout) {
  stm32_i2c_driver_t *drv = (stm32_i2c_driver_t *)self;
  stm32_i2c_soft_start(drv);
  stm32_i2c_soft_send_byte(drv, (uint8_t)dev_addr); // Write address
  if (stm32_i2c_soft_wait_ack(drv)) {
    stm32_i2c_soft_stop(drv);
    return -1;
  }

  for (uint16_t i = 0; i < size; i++) {
    stm32_i2c_soft_send_byte(drv, data[i]);
    if (stm32_i2c_soft_wait_ack(drv)) {
      stm32_i2c_soft_stop(drv);
      return -1;
    }
  }

  stm32_i2c_soft_stop(drv);
  return 0;
}

static int stm32_soft_i2c_master_receive(i2c_driver_t *self, uint16_t dev_addr,
                                         uint8_t *buffer, uint16_t size,
                                         uint32_t timeout) {
  stm32_i2c_driver_t *drv = (stm32_i2c_driver_t *)self;
  stm32_i2c_soft_start(drv);
  stm32_i2c_soft_send_byte(drv, (uint8_t)dev_addr | 0x01); // Read address
  if (stm32_i2c_soft_wait_ack(drv)) {
    stm32_i2c_soft_stop(drv);
    return -1;
  }

  for (uint16_t i = 0; i < size; i++) {
    buffer[i] = stm32_i2c_soft_read_byte(drv, (i == size - 1) ? 0 : 1);
  }

  stm32_i2c_soft_stop(drv);
  return 0;
}

#ifdef USE_I2C_MEM
static int stm32_soft_i2c_mem_write(i2c_driver_t *self, uint16_t dev_addr,
                                    uint16_t mem_addr, uint16_t mem_addr_size,
                                    uint8_t *data, uint16_t size,
                                    uint32_t timeout) {
  stm32_i2c_driver_t *drv = (stm32_i2c_driver_t *)self;
  stm32_i2c_soft_start(drv);
  stm32_i2c_soft_send_byte(drv, (uint8_t)dev_addr);
  if (stm32_i2c_soft_wait_ack(drv)) {
    stm32_i2c_soft_stop(drv);
    return -1;
  }

  // 发送寄存器地址
  if (mem_addr_size == 2) {
    stm32_i2c_soft_send_byte(drv, (uint8_t)(mem_addr >> 8));
    if (stm32_i2c_soft_wait_ack(drv)) {
      stm32_i2c_soft_stop(drv);
      return -1;
    }
  }
  stm32_i2c_soft_send_byte(drv, (uint8_t)(mem_addr & 0xFF));
  if (stm32_i2c_soft_wait_ack(drv)) {
    stm32_i2c_soft_stop(drv);
    return -1;
  }

  for (uint16_t i = 0; i < size; i++) {
    stm32_i2c_soft_send_byte(drv, data[i]);
    if (stm32_i2c_soft_wait_ack(drv)) {
      stm32_i2c_soft_stop(drv);
      return -1;
    }
  }

  stm32_i2c_soft_stop(drv);
  return 0;
}

static int stm32_soft_i2c_mem_read(i2c_driver_t *self, uint16_t dev_addr,
                                   uint16_t mem_addr, uint16_t mem_addr_size,
                                   uint8_t *buffer, uint16_t size,
                                   uint32_t timeout) {
  stm32_i2c_driver_t *drv = (stm32_i2c_driver_t *)self;

  // 写寄存器地址
  stm32_i2c_soft_start(drv);
  stm32_i2c_soft_send_byte(drv, (uint8_t)dev_addr);
  if (stm32_i2c_soft_wait_ack(drv)) {
    stm32_i2c_soft_stop(drv);
    return -1;
  }

  if (mem_addr_size == 2) {
    stm32_i2c_soft_send_byte(drv, (uint8_t)(mem_addr >> 8));
    if (stm32_i2c_soft_wait_ack(drv)) {
      stm32_i2c_soft_stop(drv);
      return -1;
    }
  }
  stm32_i2c_soft_send_byte(drv, (uint8_t)(mem_addr & 0xFF));
  if (stm32_i2c_soft_wait_ack(drv)) {
    stm32_i2c_soft_stop(drv);
    return -1;
  }

  // 重复起始信号读数据
  stm32_i2c_soft_start(drv);
  stm32_i2c_soft_send_byte(drv, (uint8_t)dev_addr | 0x01);
  if (stm32_i2c_soft_wait_ack(drv)) {
    stm32_i2c_soft_stop(drv);
    return -1;
  }

  for (uint16_t i = 0; i < size; i++) {
    buffer[i] = stm32_i2c_soft_read_byte(drv, (i == size - 1) ? 0 : 1);
  }

  stm32_i2c_soft_stop(drv);
  return 0;
}
#endif

static const i2c_driver_ops_t stm32_soft_i2c_ops = {
    .master_transmit = stm32_soft_i2c_master_transmit,
    .master_receive = stm32_soft_i2c_master_receive,
#ifdef USE_I2C_MEM
    .mem_write = stm32_soft_i2c_mem_write,
    .mem_read = stm32_soft_i2c_mem_read,
#endif
};

i2c_driver_t *
stm32_i2c_soft_driver_create(const stm32_i2c_soft_config_t *config) {
  if (config == NULL) {
    return NULL;
  }
  stm32_i2c_driver_t *drv = (stm32_i2c_driver_t *)sys_malloc(
      SYS_MEM_INTERNAL, sizeof(stm32_i2c_driver_t));
  if (drv) {
    drv->base.ops = &stm32_soft_i2c_ops;
    drv->config.is_soft = 1;
    drv->config.resource.soft_config = (stm32_i2c_soft_config_t *)config;

    // 设置默认延时
    if (CALL_SOFT_RES(drv)->delay_us == 0) {
      CALL_SOFT_RES(drv)->delay_us = DEFAULT_I2C_DELAY_US;
    }

    // 初始化 GPIO
    stm32_i2c_soft_init(drv);
  }

  return (i2c_driver_t *)drv;
}
