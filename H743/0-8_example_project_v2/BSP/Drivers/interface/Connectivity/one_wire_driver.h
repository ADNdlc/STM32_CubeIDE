/*
 * one_wire_driver.h
 *
 *  Created on: Jan 16, 2026
 *      Author: Antigravity
 *
 *  单总线 (One-Wire) 驱动抽象接口
 */

#ifndef INTERFACE_ONE_WIRE_DRIVER_H_
#define INTERFACE_ONE_WIRE_DRIVER_H_

#include <stdint.h>

// 前向声明
typedef struct one_wire_driver_t one_wire_driver_t;

// One-Wire 驱动操作接口
typedef struct {
  // 初始化 One-Wire
  int (*init)(one_wire_driver_t *self);

  // 设置引脚模式 (0: 输入, 1: 输出)
  void (*set_mode)(one_wire_driver_t *self, uint8_t mode);

  // 写数据到总线 (通常是写一个位或字节)
  void (*write)(one_wire_driver_t *self, uint8_t data);

  // 从总线读数据 (通常是读一个位或字节)
  uint8_t (*read)(one_wire_driver_t *self);

  // 释放总线/空闲状态
  void (*release)(one_wire_driver_t *self);

} one_wire_driver_ops_t;

// One-Wire 驱动基类
struct one_wire_driver_t {
  const one_wire_driver_ops_t *ops;
};

// 辅助宏，方便调用
#define ONE_WIRE_INIT(drv) ((drv)->ops->init(drv))
#define ONE_WIRE_SET_MODE(drv, mode) ((drv)->ops->set_mode(drv, mode))
#define ONE_WIRE_WRITE(drv, data) ((drv)->ops->write(drv, data))
#define ONE_WIRE_READ(drv) ((drv)->ops->read(drv))
#define ONE_WIRE_RELEASE(drv) ((drv)->ops->release(drv))

#endif /* INTERFACE_ONE_WIRE_DRIVER_H_ */
