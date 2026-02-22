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
  // 1. 获取瞬时值
  int (*read_instant)(PowerMonitor_Dev_t *dev, Power_Instant_Data_t *data);
  // 2. 获取累计值
  int (*read_accumulated)(PowerMonitor_Dev_t *dev,
                          Power_Accumulated_Data_t *data);
  // 3. 重置累计
  int (*reset_counters)(PowerMonitor_Dev_t *dev);
  // 4. 报警设置 (可选)
  int (*set_over_current_limit)(PowerMonitor_Dev_t *dev, float limit_mA);

} PowerMonitor_Ops_t;

typedef struct PowerMonitor_driver_t {
  const PowerMonitor_Ops_t *ops;
} PowerMonitor_driver_t;

// 辅助宏
#define PM_READ_INSTANT(dev, data) (dev)->ops->read_instant(dev, data)
#define PM_READ_ACCUMULATED(dev, data) (dev)->ops->read_accumulated(dev, data)
#define PM_RESET(dev) (dev)->ops->reset_counters(dev)
#define PM_SET_OVER_CURRENT_LIMIT(dev, limit_mA)                               \
  ((dev)->ops->set_over_current_limit                                          \
       ? (dev)->ops->set_over_current_limit(dev, limit_mA)                     \
       : -1)

#endif /* DRIVERS_INTERFACE_DEVICE_POWER_MONITOR_DRIVER_H_ */
