/*
 * thsensor_driver.h
 *
 *  Created on: Feb 9, 2026
 *      Author: Antigravity
 *
 *  温湿度传感器抽象接口层
 */

#ifndef BSP_DRIVERS_INTERFACE_SENSOR_THSENSOR_DRIVER_H_
#define BSP_DRIVERS_INTERFACE_SENSOR_THSENSOR_DRIVER_H_

#include <stdint.h>

// 前向声明
typedef struct humiture_driver_t humiture_driver_t;

// 温湿度传感器操作接口 (虚函数表)
typedef struct {
  // 初始化传感器
  int (*init)(humiture_driver_t *self);

  // 读取原始数据 (整数型百分位)
  // temp_x10: 温度值 * 10 (例如 25.5 -> 255)
  // humi_x10: 湿度值 * 10 (例如 60.2 -> 602)
  int (*read_raw)(humiture_driver_t *self, int16_t *temp_x10, uint16_t *humi_x10);

  // 读取格式化数据 (浮点型)
  int (*read_float)(humiture_driver_t *self, float *temp, float *humi);
} humiture_driver_ops_t;

// 温湿度传感器驱动基类
struct humiture_driver_t {
  const humiture_driver_ops_t *ops;
};

// 辅助宏
#define THSENSOR_INIT(drv) ((drv)->ops->init(drv))

#define THSENSOR_READ_RAW(drv, p_temp, p_humi)                                 \
  ((drv)->ops->read_raw(drv, p_temp, p_humi))

#define THSENSOR_READ_FLOAT(drv, p_temp, p_humi)                               \
  ((drv)->ops->read_float(drv, p_temp, p_humi))

#endif /* BSP_DRIVERS_INTERFACE_SENSOR_THSENSOR_DRIVER_H_ */
