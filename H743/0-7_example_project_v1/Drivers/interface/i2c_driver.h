/*
 * i2c_driver.h
 *
 *  Created on: Dec 13, 2025
 *      Author: Antigravity
 *
 *  I2C 驱动抽象接口
 *  支持硬件 I2C 和软件模拟 I2C
 */

#ifndef INTERFACE_I2C_DRIVER_H_
#define INTERFACE_I2C_DRIVER_H_

#include <stdint.h>

// 前向声明
typedef struct i2c_driver_t i2c_driver_t;

// I2C 驱动操作接口 (虚函数表)
typedef struct {
  // 初始化 I2C
  int (*init)(i2c_driver_t *self);

  // 发送起始信号
  void (*start)(i2c_driver_t *self);

  // 发送停止信号
  void (*stop)(i2c_driver_t *self);

  // 发送一个字节
  void (*send_byte)(i2c_driver_t *self, uint8_t data);

  // 读取一个字节
  // ack: 1=发送ACK, 0=发送NACK
  uint8_t (*read_byte)(i2c_driver_t *self, uint8_t ack);

  // 等待应答信号
  // 返回: 0=收到ACK, 1=未收到ACK(超时或NACK)
  uint8_t (*wait_ack)(i2c_driver_t *self);

  // 发送 ACK 应答
  void (*ack)(i2c_driver_t *self);

  // 发送 NACK 应答
  void (*nack)(i2c_driver_t *self);

  // 写寄存器（复合操作）
  // addr: 设备地址（7位），reg: 寄存器地址，buf: 数据缓冲区，len: 数据长度
  // 返回: 0=成功, !0=失败
  int (*write_reg)(i2c_driver_t *self, uint8_t addr, uint16_t reg, uint8_t *buf,
                   uint8_t len);

  // 读寄存器（复合操作）
  // addr: 设备地址（7位），reg: 寄存器地址，buf: 数据缓冲区，len: 数据长度
  // 返回: 0=成功, !0=失败
  int (*read_reg)(i2c_driver_t *self, uint8_t addr, uint16_t reg, uint8_t *buf,
                  uint8_t len);
} i2c_driver_ops_t;

// I2C 驱动基类
struct i2c_driver_t {
  const i2c_driver_ops_t *ops;
};

// 辅助宏，方便调用
#define I2C_INIT(drv) ((drv)->ops->init(drv))
#define I2C_START(drv) ((drv)->ops->start(drv))
#define I2C_STOP(drv) ((drv)->ops->stop(drv))
#define I2C_SEND_BYTE(drv, data) ((drv)->ops->send_byte(drv, data))
#define I2C_READ_BYTE(drv, ack) ((drv)->ops->read_byte(drv, ack))
#define I2C_WAIT_ACK(drv) ((drv)->ops->wait_ack(drv))
#define I2C_ACK(drv) ((drv)->ops->ack(drv))
#define I2C_NACK(drv) ((drv)->ops->nack(drv))
#define I2C_WRITE_REG(drv, addr, reg, buf, len)                                \
  ((drv)->ops->write_reg(drv, addr, reg, buf, len))
#define I2C_READ_REG(drv, addr, reg, buf, len)                                 \
  ((drv)->ops->read_reg(drv, addr, reg, buf, len))

#endif /* INTERFACE_I2C_DRIVER_H_ */
