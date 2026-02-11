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
#define BH1750_CMD_ONE_TIME_H_RES 0x20 // 一次性 H 分辨率模式 (1lx, 120ms),测试后自动进入断电模式
#define BH1750_CMD_ONE_TIME_H_RES2 0x21// 一次性 H 分辨率模式2 (0.5lx, 120ms),测试后自动进入断电模式
#define BH1750_CMD_ONE_TIME_L_RES 0x23 // 一次性 L 分辨率模式 (4lx, 16ms),测试后自动进入断电模式



#endif /* DEVICE_BH1750_BH1750_DRIVER_H_ */
