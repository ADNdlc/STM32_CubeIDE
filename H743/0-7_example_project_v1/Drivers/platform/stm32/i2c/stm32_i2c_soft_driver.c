/*
 * stm32_i2c_soft_driver.c
 *
 *  Created on: Dec 13, 2025
 *      Author: Antigravity
 *
 *  STM32 软件模拟 I2C 驱动实现
 *  基于 GPIO 位操作实现软件 I2C
 */

#include "stm32_i2c_soft_driver.h"
#include "sys.h"
#include <stdlib.h>

// 默认延时（微秒）
#define DEFAULT_I2C_DELAY_US 2

//==============================================================================
// 私有辅助函数
//==============================================================================

/**
 * @brief I2C 延时
 */
static inline void i2c_delay(stm32_i2c_soft_driver_t *drv) {
  sys_delay_us(drv->config.delay_us);
}

/**
 * @brief 设置 SCL 电平
 */
static inline void i2c_scl_set(stm32_i2c_soft_driver_t *drv, uint8_t value) {
  HAL_GPIO_WritePin(drv->config.scl_port, drv->config.scl_pin,
                    value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief 设置 SDA 电平
 */
static inline void i2c_sda_set(stm32_i2c_soft_driver_t *drv, uint8_t value) {
  HAL_GPIO_WritePin(drv->config.sda_port, drv->config.sda_pin,
                    value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief 读取 SDA 电平
 */
static inline uint8_t i2c_sda_read(stm32_i2c_soft_driver_t *drv) {
  return (uint8_t)HAL_GPIO_ReadPin(drv->config.sda_port, drv->config.sda_pin);
}

//==============================================================================
// I2C 驱动接口实现
//==============================================================================

/**
 * @brief 初始化 I2C GPIO
 */
static int stm32_i2c_soft_init(i2c_driver_t *self) {
  stm32_i2c_soft_driver_t *drv = (stm32_i2c_soft_driver_t *)self;
  GPIO_InitTypeDef gpio_init;

  if (drv->initialized) {
    return 0;
  }

  // 使能 GPIO 时钟（假设已在外部使能，这里只做引脚配置）
  // SCL 配置为开漏输出（用于I2C时钟线需要支持时钟拉伸）
  gpio_init.Pin = drv->config.scl_pin;
  gpio_init.Mode = GPIO_MODE_OUTPUT_OD;
  gpio_init.Pull = GPIO_PULLUP;
  gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(drv->config.scl_port, &gpio_init);

  // SDA 配置为开漏输出（用于双向数据传输）
  gpio_init.Pin = drv->config.sda_pin;
  gpio_init.Mode = GPIO_MODE_OUTPUT_OD;
  gpio_init.Pull = GPIO_PULLUP;
  gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(drv->config.sda_port, &gpio_init);

  // 初始状态：空闲（SCL=1, SDA=1）
  i2c_sda_set(drv, 1);
  i2c_scl_set(drv, 1);

  drv->initialized = 1;
  return 0;
}

/**
 * @brief 产生 I2C 起始信号
 * START: SCL 为高时，SDA 从高变低
 */
static void stm32_i2c_soft_start(i2c_driver_t *self) {
  stm32_i2c_soft_driver_t *drv = (stm32_i2c_soft_driver_t *)self;

  i2c_sda_set(drv, 1);
  i2c_scl_set(drv, 1);
  i2c_delay(drv);
  i2c_sda_set(drv, 0); // START: SDA 从高变低
  i2c_delay(drv);
  i2c_scl_set(drv, 0); // 钳住总线，准备发送/接收数据
  i2c_delay(drv);
}

/**
 * @brief 产生 I2C 停止信号
 * STOP: SCL 为高时，SDA 从低变高
 */
static void stm32_i2c_soft_stop(i2c_driver_t *self) {
  stm32_i2c_soft_driver_t *drv = (stm32_i2c_soft_driver_t *)self;

  i2c_sda_set(drv, 0); // 确保 SDA 为低
  i2c_delay(drv);
  i2c_scl_set(drv, 1); // SCL 变高
  i2c_delay(drv);
  i2c_sda_set(drv, 1); // STOP: SDA 从低变高
  i2c_delay(drv);
}

/**
 * @brief 发送一个字节
 * MSB 先发
 */
static void stm32_i2c_soft_send_byte(i2c_driver_t *self, uint8_t data) {
  stm32_i2c_soft_driver_t *drv = (stm32_i2c_soft_driver_t *)self;

  for (uint8_t i = 0; i < 8; i++) {
    i2c_sda_set(drv, (data & 0x80) >> 7); // 发送最高位
    i2c_delay(drv);
    i2c_scl_set(drv, 1); // SCL 上升沿锁存数据
    i2c_delay(drv);
    i2c_scl_set(drv, 0); // 准备下一位
    data <<= 1;          // 左移，准备发送下一位
  }
  // 释放 SDA
  i2c_sda_set(drv, 1);
}

/**
 * @brief 读取一个字节
 * @param ack 1=发送ACK, 0=发送NACK
 */
static uint8_t stm32_i2c_soft_read_byte(i2c_driver_t *self, uint8_t ack) {
  stm32_i2c_soft_driver_t *drv = (stm32_i2c_soft_driver_t *)self;
  uint8_t receive = 0;

  // 释放 SDA，准备读取
  i2c_sda_set(drv, 1);

  for (uint8_t i = 0; i < 8; i++) {
    receive <<= 1;       // 左移（高位先接收）
    i2c_scl_set(drv, 1); // SCL 上升沿读取数据
    i2c_delay(drv);
    if (i2c_sda_read(drv)) {
      receive++; // 读取到高电平
    }
    i2c_scl_set(drv, 0);
    i2c_delay(drv);
  }

  // 发送应答
  if (ack) {
    // 发送 ACK (SDA=0)
    i2c_sda_set(drv, 0);
    i2c_delay(drv);
    i2c_scl_set(drv, 1);
    i2c_delay(drv);
    i2c_scl_set(drv, 0);
    i2c_delay(drv);
    i2c_sda_set(drv, 1); // 释放 SDA
    i2c_delay(drv);
  } else {
    // 发送 NACK (SDA=1)
    i2c_sda_set(drv, 1);
    i2c_delay(drv);
    i2c_scl_set(drv, 1);
    i2c_delay(drv);
    i2c_scl_set(drv, 0);
    i2c_delay(drv);
  }

  return receive;
}

/**
 * @brief 等待应答信号
 * @return 0=收到ACK, 1=超时或NACK
 */
static uint8_t stm32_i2c_soft_wait_ack(i2c_driver_t *self) {
  stm32_i2c_soft_driver_t *drv = (stm32_i2c_soft_driver_t *)self;
  uint8_t wait_time = 0;
  uint8_t rack = 0;

  // 释放 SDA（主机释放，等待从机应答）
  i2c_sda_set(drv, 1);
  i2c_delay(drv);
  i2c_scl_set(drv, 1); // SCL=1，此时从机可以返回 ACK
  i2c_delay(drv);

  // 等待 SDA 变低（ACK）
  while (i2c_sda_read(drv)) {
    wait_time++;
    if (wait_time > 250) {
      stm32_i2c_soft_stop(self);
      rack = 1;
      break;
    }
    i2c_delay(drv);
  }

  i2c_scl_set(drv, 0); // SCL=0，结束 ACK 检查
  i2c_delay(drv);

  return rack;
}

/**
 * @brief 发送 ACK 应答
 */
static void stm32_i2c_soft_ack(i2c_driver_t *self) {
  stm32_i2c_soft_driver_t *drv = (stm32_i2c_soft_driver_t *)self;

  // ACK: SCL 0->1 时 SDA=0
  i2c_sda_set(drv, 0);
  i2c_delay(drv);
  i2c_scl_set(drv, 1);
  i2c_delay(drv);
  i2c_scl_set(drv, 0);
  i2c_delay(drv);
  i2c_sda_set(drv, 1); // 释放 SDA
  i2c_delay(drv);
}

/**
 * @brief 发送 NACK 应答
 */
static void stm32_i2c_soft_nack(i2c_driver_t *self) {
  stm32_i2c_soft_driver_t *drv = (stm32_i2c_soft_driver_t *)self;

  // NACK: SCL 0->1 时 SDA=1
  i2c_sda_set(drv, 1);
  i2c_delay(drv);
  i2c_scl_set(drv, 1);
  i2c_delay(drv);
  i2c_scl_set(drv, 0);
  i2c_delay(drv);
}

/**
 * @brief 写寄存器（16位寄存器地址）
 * @param addr 设备地址（7位，会转换为8位写地址）
 * @param reg 16位寄存器地址
 * @param buf 数据缓冲区
 * @param len 数据长度
 * @return 0=成功, !0=失败
 */
static int stm32_i2c_soft_write_reg(i2c_driver_t *self, uint8_t addr,
                                    uint16_t reg, uint8_t *buf, uint8_t len) {
  stm32_i2c_soft_start(self);
  stm32_i2c_soft_send_byte(self, addr << 1); // 写地址
  if (stm32_i2c_soft_wait_ack(self)) {
    return 1;
  }

  stm32_i2c_soft_send_byte(self, reg >> 8); // 寄存器高8位
  if (stm32_i2c_soft_wait_ack(self)) {
    return 1;
  }

  stm32_i2c_soft_send_byte(self, reg & 0xFF); // 寄存器低8位
  if (stm32_i2c_soft_wait_ack(self)) {
    return 1;
  }

  for (uint8_t i = 0; i < len; i++) {
    stm32_i2c_soft_send_byte(self, buf[i]);
    if (stm32_i2c_soft_wait_ack(self)) {
      return 1;
    }
  }

  stm32_i2c_soft_stop(self);
  return 0;
}

/**
 * @brief 读寄存器（16位寄存器地址）
 * @param addr 设备地址（7位，会转换为8位读/写地址）
 * @param reg 16位寄存器地址
 * @param buf 数据缓冲区
 * @param len 数据长度
 * @return 0=成功, !0=失败
 */
static int stm32_i2c_soft_read_reg(i2c_driver_t *self, uint8_t addr,
                                   uint16_t reg, uint8_t *buf, uint8_t len) {
  // 先发送寄存器地址
  stm32_i2c_soft_start(self);
  stm32_i2c_soft_send_byte(self, addr << 1); // 写地址
  if (stm32_i2c_soft_wait_ack(self)) {
    return 1;
  }

  stm32_i2c_soft_send_byte(self, reg >> 8); // 寄存器高8位
  if (stm32_i2c_soft_wait_ack(self)) {
    return 1;
  }

  stm32_i2c_soft_send_byte(self, reg & 0xFF); // 寄存器低8位
  if (stm32_i2c_soft_wait_ack(self)) {
    return 1;
  }

  // 重新启动，切换到读模式
  stm32_i2c_soft_start(self);
  stm32_i2c_soft_send_byte(self, (addr << 1) | 0x01); // 读地址
  if (stm32_i2c_soft_wait_ack(self)) {
    return 1;
  }

  // 读取数据
  for (uint8_t i = 0; i < len; i++) {
    buf[i] = stm32_i2c_soft_read_byte(self, (i == (len - 1)) ? 0 : 1);
  }

  stm32_i2c_soft_stop(self);
  return 0;
}

//==============================================================================
// 虚函数表
//==============================================================================

static const i2c_driver_ops_t stm32_i2c_soft_ops = {
    .init = stm32_i2c_soft_init,
    .start = stm32_i2c_soft_start,
    .stop = stm32_i2c_soft_stop,
    .send_byte = stm32_i2c_soft_send_byte,
    .read_byte = stm32_i2c_soft_read_byte,
    .wait_ack = stm32_i2c_soft_wait_ack,
    .ack = stm32_i2c_soft_ack,
    .nack = stm32_i2c_soft_nack,
    .write_reg = stm32_i2c_soft_write_reg,
    .read_reg = stm32_i2c_soft_read_reg,
};

//==============================================================================
// 公共函数
//==============================================================================

stm32_i2c_soft_driver_t *
stm32_i2c_soft_create(const stm32_i2c_soft_config_t *config) {
  if (config == NULL) {
    return NULL;
  }

  stm32_i2c_soft_driver_t *drv =
      (stm32_i2c_soft_driver_t *)malloc(sizeof(stm32_i2c_soft_driver_t));
  if (drv) {
    drv->base.ops = &stm32_i2c_soft_ops;
    drv->config = *config;
    drv->initialized = 0;

    // 设置默认延时
    if (drv->config.delay_us == 0) {
      drv->config.delay_us = DEFAULT_I2C_DELAY_US;
    }
  }

  return drv;
}

void stm32_i2c_soft_destroy(stm32_i2c_soft_driver_t *drv) {
  if (drv) {
    free(drv);
  }
}
