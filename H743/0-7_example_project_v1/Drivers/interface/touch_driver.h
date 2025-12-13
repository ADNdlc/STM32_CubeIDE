/*
 * touch_driver.h
 *
 *  Created on: Dec 13, 2025
 *      Author: Antigravity
 *
 *  触摸屏驱动抽象接口
 */

#ifndef INTERFACE_TOUCH_DRIVER_H_
#define INTERFACE_TOUCH_DRIVER_H_

#include <stdint.h>

// 最大触摸点数
#define TOUCH_MAX_POINTS 10

// 触摸点数据
typedef struct {
  uint16_t x;
  uint16_t y;
} touch_point_t;

// 触摸数据
typedef struct {
  uint8_t count;                          // 触摸点数量
  touch_point_t points[TOUCH_MAX_POINTS]; // 触摸点坐标
} touch_data_t;

// 前向声明
typedef struct touch_driver_t touch_driver_t;

// 触摸屏驱动操作接口
typedef struct {
  // 初始化触摸屏
  // 返回: 0=成功, !0=失败
  int (*init)(touch_driver_t *self);

  // 扫描触摸屏
  // data: 输出触摸数据
  // 返回: 0=无触摸, 1=有触摸, <0=错误
  int (*scan)(touch_driver_t *self, touch_data_t *data);

  // 获取支持的最大触摸点数
  uint8_t (*get_max_points)(touch_driver_t *self);

  // 获取设备ID字符串
  const char *(*get_device_id)(touch_driver_t *self);
} touch_driver_ops_t;

// 触摸屏驱动基类
struct touch_driver_t {
  const touch_driver_ops_t *ops;
};

// 辅助宏
#define TOUCH_INIT(drv) ((drv)->ops->init(drv))
#define TOUCH_SCAN(drv, data) ((drv)->ops->scan(drv, data))
#define TOUCH_GET_MAX_POINTS(drv) ((drv)->ops->get_max_points(drv))
#define TOUCH_GET_DEVICE_ID(drv) ((drv)->ops->get_device_id(drv))

#endif /* INTERFACE_TOUCH_DRIVER_H_ */
