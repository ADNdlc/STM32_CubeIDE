/*
 * ina219_driver.h
 *
 *  Created on: Feb 20, 2026
 *      Author: Antigravity
 */

#ifndef DRIVERS_DEVICE_INA219_INA219_DRIVER_H_
#define DRIVERS_DEVICE_INA219_INA219_DRIVER_H_

#include "PowerMonitor_driver.h"
#include "i2c_driver.h"
#include "timer_driver.h"
#include <stdint.h>

// I2C 地址配置
#define INA219_I2C_ADDRESS_CONF_0 (u8)(0x40 << 1) // A0 = GND, A1 = GND
#define INA219_I2C_ADDRESS_CONF_1 (u8)(0x41 << 1) // A0 = VS+, A1 = GND
#define INA219_I2C_ADDRESS_CONF_2 (u8)(0x42 << 1) // A0 = SDA, A1 = GND
#define INA219_I2C_ADDRESS_CONF_3 (u8)(0x43 << 1) // A0 = SCL, A1 = GND
#define INA219_I2C_ADDRESS_CONF_4 (u8)(0x44 << 1) // A0 = GND, A1 = VS+
#define INA219_I2C_ADDRESS_CONF_5 (u8)(0x45 << 1) // A0 = VS+, A1 = VS+
#define INA219_I2C_ADDRESS_CONF_6 (u8)(0x46 << 1) // A0 = SDA, A1 = VS+
#define INA219_I2C_ADDRESS_CONF_7 (u8)(0x47 << 1) // A0 = SCL, A1 = VS+
#define INA219_I2C_ADDRESS_CONF_8 (u8)(0x48 << 1) // A0 = GND, A1 = SDA
#define INA219_I2C_ADDRESS_CONF_9 (u8)(0x49 << 1) // A0 = VS+, A1 = SDA
#define INA219_I2C_ADDRESS_CONF_A (u8)(0x4A << 1) // A0 = SDA, A1 = SDA
#define INA219_I2C_ADDRESS_CONF_B (u8)(0x4B << 1) // A0 = SCL, A1 = SDA
#define INA219_I2C_ADDRESS_CONF_C (u8)(0x4C << 1) // A0 = GND, A1 = SCL
#define INA219_I2C_ADDRESS_CONF_D (u8)(0x4D << 1) // A0 = VS+, A1 = SCL
#define INA219_I2C_ADDRESS_CONF_E (u8)(0x4E << 1) // A0 = SDA, A1 = SCL
#define INA219_I2C_ADDRESS_CONF_F (u8)(0x4F << 1) // A0 = SCL, A1 = SCL
#define INA219_I2C_ADDRESS INA219_I2C_ADDRESS_CONF_0

// 内部状态机
typedef enum {
  INA219_STATE_IDLE = 0,
  INA219_STATE_WRITE_PTR_BUSV, // 设置总线电压寄存器地址
  INA219_STATE_READ_BUSV,      // 读取总线电压
  INA219_STATE_WRITE_PTR_CURR, // 设置电流寄存器地址
  INA219_STATE_READ_CURR,      // 读取电流
  INA219_STATE_WRITE_PTR_POW,  // 设置功率寄存器地址
  INA219_STATE_READ_POW,       // 读取功率
  INA219_STATE_CALCULATE       // 计算数据
} ina219_state_t;

typedef struct {
  uint16_t dev_addr;        // I2C 设备地址 (8-bit)
  float shunt_resistor_ohm; // 采样电阻值 (欧姆)
  float max_current_A;      // 最大预期电流 (用于计算校准值)
} ina219_config_t;

typedef struct {
  PowerMonitor_driver_t base; // 实现接口的功能
  // 依赖
  i2c_driver_t *i2c_driver; // 通信依赖
  timer_driver_t *timer;    // ina219没有硬件积分，因此需要定时采样软件累计
  // 配置
  ina219_config_t config; // 硬件参数
  // 核心计算参数
  float current_lsb;  // 电流解析度 (mA/bit)
  float power_lsb;    // 功率解析度 (mW/bit)
  uint16_t cal_value; // 校准寄存器值
  // 运行状态
  ina219_state_t fsm_state;
  uint8_t i2c_buffer[4]; // I2C 异步通信缓冲区
  // 数据缓存
  Power_Instant_Data_t instant_data;
  Power_Accumulated_Data_t accumulated_data;
} ina219_driver_t;

/**
 * @brief 创建 INA219 驱动实例
 *
 * @param i2c I2C 驱动指针
 * @param config 配置参数
 * @return PowerMonitor_Dev_t*
 */
PowerMonitor_driver_t *ina219_create(i2c_driver_t *i2c, timer_driver_t *timer,
                                     ina219_config_t *config);

#endif /* DRIVERS_DEVICE_INA219_INA219_DRIVER_H_ */
