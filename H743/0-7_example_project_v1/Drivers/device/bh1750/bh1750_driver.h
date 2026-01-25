/*
 * bh1750_driver.h
 *
 *  Created on: Jan 25, 2026
 *      Author: Antigravity
 *
 *  BH1750 数字光照传感器驱动
 *  适配 i2c_driver 接口
 */

#ifndef DEVICE_BH1750_BH1750_DRIVER_H_
#define DEVICE_BH1750_BH1750_DRIVER_H_

#include "i2c_driver.h"
#include <stdint.h>

// BH1750 指令定义
#define BH1750_CMD_POWER_DOWN 0x00
#define BH1750_CMD_POWER_ON 0x01
#define BH1750_CMD_RESET 0x07
#define BH1750_CMD_CH_RES 0x10  // 连续 H 分辨率模式 (1lx, 120ms)
#define BH1750_CMD_CH_RES2 0x11 // 连续 H 分辨率模式2 (0.5lx, 120ms)
#define BH1750_CMD_CL_RES 0x13  // 连续 L 分辨率模式 (4lx, 16ms)

// BH1750 驱动配置
typedef struct {
  i2c_driver_t *i2c; // I2C 驱动实例
  uint8_t i2c_addr;  // I2C 7位地址
} bh1750_config_t;

// BH1750 驱动结构体
typedef struct {
  bh1750_config_t config; // 配置
  float lux;              // 当前光照强度 (Lux)
  uint8_t initialized;    // 初始化标志
} bh1750_driver_t;

/**
 * @brief 初始化 BH1750 驱动
 * @param self 驱动实例指针
 * @param config 配置参数
 * @return 0: 成功, !0: 失败
 */
int bh1750_init(bh1750_driver_t *self, const bh1750_config_t *config);

/**
 * @brief 读取光照强度
 * @param self 驱动实例指针
 * @return 0: 成功, !0: 失败
 */
int bh1750_read_lux(bh1750_driver_t *self);

#endif /* DEVICE_BH1750_BH1750_DRIVER_H_ */
