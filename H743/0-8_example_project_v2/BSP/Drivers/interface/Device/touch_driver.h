/*
 * touch_driver.h
 *
 *  Created on: Feb 12, 2026
 *      Author: 12114
 */

#ifndef DRIVERS_INTERFACE_DEVICE_TOUCH_DRIVER_H_
#define DRIVERS_INTERFACE_DEVICE_TOUCH_DRIVER_H_

#include <stdbool.h>
#include <stdint.h>

// 最大支持触点数 (电容屏通常支持 5-10 点，电阻屏 1 点)
#define TOUCH_MAX_POINTS 10

// 触摸状态枚举
typedef enum {
  TOUCH_STATE_RELEASED = 0,
  TOUCH_STATE_PRESSED = 1
} Touch_State_t;

// 单个触点的坐标结构
typedef struct {
  uint16_t x; // X 坐标
  uint16_t y; // Y 坐标
} Touch_Point_t;

// 触摸屏数据帧 (由驱动层填充，App层读取)
typedef struct {
  Touch_State_t state; // 输入状态 (只要有一个点按下即为 PRESSED)
  uint8_t point_count; // 当前有效触点数量
  Touch_Point_t points[TOUCH_MAX_POINTS]; // 触点数组
} Touch_Data_t;
// 前向声明
typedef struct touch_driver_t touch_driver_t;

typedef struct {
  int (*init)(touch_driver_t *self);
  Touch_Data_t *(*get_touch_data)(touch_driver_t *self);
  int (*scan)(touch_driver_t *self); // 更新坐标
} touch_driver_ops_t;

// 触摸屏设备对象 (句柄)
struct touch_driver_t {
  const touch_driver_ops_t *ops;
  Touch_Data_t touch_data;
};

// 辅助宏
#define TOUCH_INIT(drv) (drv)->ops->init(drv)
#define TOUCH_SCAN(drv) (drv)->ops->scan(drv)
#define TOUCH_GET_DATA(drv) (drv)->ops->get_touch_data(drv)

#endif /* DRIVERS_INTERFACE_DEVICE_TOUCH_DRIVER_H_ */
