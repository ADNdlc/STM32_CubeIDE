/*
 * gt9xxx_touch_driver.h
 *
 *  Created on: Dec 13, 2025
 *      Author: Antigravity
 *
 *  GT9xxx 系列触摸屏驱动（平台无关）
 *  支持 GT911、GT9147、GT1158、GT9271 等触摸芯片
 *  依赖 i2c_driver 和 gpio_driver 抽象接口
 */

#ifndef DEVICE_GT9XXX_TOUCH_DRIVER_H_
#define DEVICE_GT9XXX_TOUCH_DRIVER_H_

#include "gpio_driver.h"
#include "i2c_driver.h"
#include "touch_driver.h"

//==============================================================================
// GT9XXX 寄存器定义
//==============================================================================

#define GT9XXX_CTRL_REG 0x8040  // 控制寄存器
#define GT9XXX_CFGS_REG 0x8047  // 配置起始地址寄存器
#define GT9XXX_CHECK_REG 0x80FF // 校验和寄存器
#define GT9XXX_PID_REG 0x8140   // 产品ID寄存器

#define GT9XXX_GSTID_REG 0x814E // 触摸状态寄存器
#define GT9XXX_TP1_REG 0x8150   // 第1个触摸点数据地址
#define GT9XXX_TP2_REG 0x8158   // 第2个触摸点数据地址
#define GT9XXX_TP3_REG 0x8160   // 第3个触摸点数据地址
#define GT9XXX_TP4_REG 0x8168   // 第4个触摸点数据地址
#define GT9XXX_TP5_REG 0x8170   // 第5个触摸点数据地址
#define GT9XXX_TP6_REG 0x8178   // 第6个触摸点数据地址
#define GT9XXX_TP7_REG 0x8180   // 第7个触摸点数据地址
#define GT9XXX_TP8_REG 0x8188   // 第8个触摸点数据地址
#define GT9XXX_TP9_REG 0x8190   // 第9个触摸点数据地址
#define GT9XXX_TP10_REG 0x8198  // 第10个触摸点数据地址

//==============================================================================
// I2C 地址定义
//==============================================================================

// GT9xxx 支持两种 I2C 地址，由复位时序决定
// 地址模式1: 0x14 (7位) -> 0x28/0x29 (8位读写)
// 地址模式2: 0x5D (7位) -> 0xBA/0xBB (8位读写)
typedef enum {
  GT9XXX_ADDR_0x14 = 0x14, // 7位地址 0x14 (复位时 INT=HIGH)
  GT9XXX_ADDR_0x5D = 0x5D, // 7位地址 0x5D (复位时 INT=LOW)
} gt9xxx_addr_mode_t;

//==============================================================================
// GT9xxx 驱动配置和结构体
//==============================================================================

// GT9xxx 配置
typedef struct {
  void *i2c_conf;       	// I2C 驱动配置
  void *rst_gpio_conf;      // RST 引脚配置
  void *int_gpio_conf;      // INT 引脚配置
  gt9xxx_addr_mode_t addr_mode_conf; // I2C 地址模式
} gt9xxx_config_t;

// GT9xxx 通信
typedef struct {
  i2c_driver_t *i2c;            // I2C 驱动实例
  gpio_driver_t *rst_gpio;      // RST 引脚驱动
  gpio_driver_t *int_gpio;      // INT 引脚驱动
  gt9xxx_addr_mode_t addr_mode; // I2C 地址模式
} gt9xxx_bus_t;

// GT9xxx 驱动结构体
typedef struct {
  touch_driver_t base;    // 继承自 touch_driver_t 基类
  gt9xxx_bus_t bus;    // 通信依赖
  uint8_t max_points;	  // 最大触摸数量(自动判断所以没放在config里)
  char device_id[8];      // 设备ID字符串
} gt9xxx_touch_driver_t;

//==============================================================================
// 公共函数
//==============================================================================

/**
 * @brief 创建 GT9xxx 触摸屏驱动实例
 * @param config 配置参数
 * @return 驱动实例指针，失败返回 NULL
 */
touch_driver_t *gt9xxx_touch_create(const gt9xxx_bus_t *bus);

/**
 * @brief 销毁 GT9xxx 触摸屏驱动实例
 * @param drv 驱动实例指针
 */
void gt9xxx_touch_destroy(gt9xxx_touch_driver_t *drv);

#endif /* DEVICE_GT9XXX_TOUCH_DRIVER_H_ */
