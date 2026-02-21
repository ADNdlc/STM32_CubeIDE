/*
 * power_monitor_driver.h
 *
 *  Created on: Feb 20, 2026
 *      Author: Antigravity
 */

#ifndef DRIVERS_INTERFACE_DEVICE_POWER_MONITOR_DRIVER_H_
#define DRIVERS_INTERFACE_DEVICE_POWER_MONITOR_DRIVER_H_

#include <stdint.h>

// 统一数据单位：
// 电压: mV (毫伏)
// 电流: mA (毫安)
// 功率: mW (毫瓦)
// 能量: mWh (毫瓦时)
// 电荷: mAh (毫安时)

typedef struct {
  float voltage_mV;
  float current_mA;
  float power_mW;
} Power_Instant_Data_t;

typedef struct {
  double energy_mWh;
  double charge_mAh;
} Power_Accumulated_Data_t;

// 驱动句柄前向声明
typedef struct PowerMonitor_Dev PowerMonitor_Dev_t;

typedef struct {
  // 1. 初始化
  int (*init)(PowerMonitor_Dev_t *dev);

  // 2. 获取瞬时值
  int (*read_instant)(PowerMonitor_Dev_t *dev, Power_Instant_Data_t *data);

  // 3. 获取累计值 (库仑计)
  int (*read_accumulated)(PowerMonitor_Dev_t *dev,
                          Power_Accumulated_Data_t *data);

  // 4. 重置累计
  int (*reset_counters)(PowerMonitor_Dev_t *dev);

} PowerMonitor_Ops_t;

struct PowerMonitor_Dev {
  const PowerMonitor_Ops_t *ops;
};

// 辅助宏
#define PM_INIT(dev) (dev)->ops->init(dev)
#define PM_READ_INSTANT(dev, data) (dev)->ops->read_instant(dev, data)
#define PM_READ_ACCUMULATED(dev, data) (dev)->ops->read_accumulated(dev, data)
#define PM_RESET(dev) (dev)->ops->reset_counters(dev)

#endif /* DRIVERS_INTERFACE_DEVICE_POWER_MONITOR_DRIVER_H_ */
