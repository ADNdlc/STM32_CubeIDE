/*
 * power_monitor_driver.h
 *
 *  Created on: Feb 19, 2026
 *      Author: 12114
 */

#ifndef DRIVERS_INTERFACE_DEVICE_POWER_MONITOR_DRIVER_H_
#define DRIVERS_INTERFACE_DEVICE_POWER_MONITOR_DRIVER_H_

// 统一数据单位：
// 电压: mV (毫伏)
// 电流: mA (毫安)
// 功率: mW (毫瓦)
// 能量: mWh (毫瓦时) 或 mAh (毫安时)

typedef struct {
    float voltage_mV;
    float current_mA;
    float power_mW;
    float energy_mWh; // INA229有硬件积分，INA219需要软件积分
} Power_Data_t;

// 驱动句柄
typedef struct PowerMonitor_Dev PowerMonitor_Dev_t;

typedef struct {
    // 1. 初始化 (传入配置结构体)
    int (*init)(PowerMonitor_Dev_t *dev);

    // 2. 基础读取 (获取瞬时值)
    int (*read_bus_voltage)(PowerMonitor_Dev_t *dev, float *mV);
    int (*read_current)(PowerMonitor_Dev_t *dev, float *mA);
    int (*read_power)(PowerMonitor_Dev_t *dev, float *mW);

    // 3. 批量读取 (推荐，减少总线通信开销)
    int (*read_all)(PowerMonitor_Dev_t *dev, Power_Data_t *data);

    // 4. 累积量操作 (库仑计核心)
    int (*reset_energy_counter)(PowerMonitor_Dev_t *dev); // 清零累计电量

    // 5. 报警设置 (可选)
    int (*set_over_current_limit)(PowerMonitor_Dev_t *dev, float limit_mA);
} PowerMonitor_Ops_t;

#endif /* DRIVERS_INTERFACE_DEVICE_POWER_MONITOR_DRIVER_H_ */
