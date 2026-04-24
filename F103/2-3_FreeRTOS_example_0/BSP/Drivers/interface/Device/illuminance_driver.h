/*
 * illuminance_driver.h
 *
 *  Created on: Feb 11, 2026
 *      Author: Antigravity
 *
 *  照度/光照传感器抽象接口层
 */

#ifndef BSP_DRIVERS_INTERFACE_DEVICE_ILLUMINANCE_DRIVER_H_
#define BSP_DRIVERS_INTERFACE_DEVICE_ILLUMINANCE_DRIVER_H_

#include <stdint.h>

// 前向声明
typedef struct illuminance_driver_t illuminance_driver_t;

// 光照传感器操作接口 (虚函数表)
typedef struct {
  // 初始化传感器
  int (*init)(illuminance_driver_t *self);

  // 读取照度 (Lux)
  int (*read_lux)(illuminance_driver_t *self, float *lux);
} illuminance_driver_ops_t;

// 光照传感器驱动基类
struct illuminance_driver_t {
  const illuminance_driver_ops_t *ops;
};

// 辅助宏
#define ILLUMINANCE_INIT(drv) ((drv)->ops->init(drv))

#define ILLUMINANCE_READ_LUX(drv, p_lux) ((drv)->ops->read_lux(drv, p_lux))

#endif /* BSP_DRIVERS_INTERFACE_DEVICE_ILLUMINANCE_DRIVER_H_ */
