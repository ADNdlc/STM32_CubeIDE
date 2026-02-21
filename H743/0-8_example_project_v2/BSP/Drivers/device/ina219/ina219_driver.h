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
#include "i2c_queue/i2c_queue.h"

typedef struct {
  uint16_t dev_addr;        // I2C 设备地址 (8-bit)
  float shunt_resistor_ohm; // 采样电阻值 (欧姆)
  float max_current_A;      // 最大预期电流 (用于计算校准值)
} ina219_config_t;

typedef struct {
  PowerMonitor_Dev_t base;

  // 依赖
  i2c_driver_t *i2c_driver;	// 通信
  i2c_queue_t rx_queue; // 异步接收队列

  // 配置
  ina219_config_t config;
  uint32_t cal_value;   // 校准寄存器值
  float current_lsb_mA; // 电流分辨率
  float power_lsb_mW;   // 功率分辨率

  // 状态
  uint32_t last_process_tick;
  double accumulated_energy_mWs;
  double accumulated_charge_mAs;

} ina219_driver_t;

/**
 * @brief 创建 INA219 驱动实例
 *
 * @param i2c I2C 驱动指针
 * @param config 配置参数
 * @return PowerMonitor_Dev_t*
 */
PowerMonitor_Dev_t *ina219_create(i2c_driver_t *i2c, ina219_config_t *config);

/**
 * @brief 请求一次异步采样 (应在定时器中调用)
 * 默认读取电流寄存器进行累积
 * @param dev
 * @return int 0:成功, -1:失败
 */
int ina219_request_sample(PowerMonitor_Dev_t *dev);

/**
 * @brief 处理采样数据并更新累积值 (应在主循环或低优先级任务中调用)
 *
 * @param dev
 */
void ina219_process_data(PowerMonitor_Dev_t *dev);

#endif /* DRIVERS_DEVICE_INA219_INA219_DRIVER_H_ */
