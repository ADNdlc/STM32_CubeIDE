#ifndef BSP_DRIVERS_INTERFACE_DEVICE_ABSOLUTE_ENCODER_DRIVER_H_
#define BSP_DRIVERS_INTERFACE_DEVICE_ABSOLUTE_ENCODER_DRIVER_H_

#include <stdint.h>

// 前向声明
typedef struct absolute_encoder_driver_t absolute_encoder_driver_t;

// 绝对值编码器操作虚表
typedef struct {
  // 获取原始角度值
  uint32_t (*get_raw_angle)(absolute_encoder_driver_t *self);
  // 获取弧度值 (0 - 2PI)
  float (*get_angle)(absolute_encoder_driver_t *self);
  // 获取AGC值（0-255）,磁场检测
  uint8_t (*get_acg)(absolute_encoder_driver_t *self);
  // 检查磁铁状态 (1: 正常, 0: 异常)
  uint8_t (*get_status)(absolute_encoder_driver_t *self);
} absolute_encoder_driver_ops_t;

// 绝对值编码器基类结构体
struct absolute_encoder_driver_t {
  const absolute_encoder_driver_ops_t *ops;
};

// 定义快捷调用宏
#define ENCODER_GET_RAW(drv) ((drv)->ops->get_raw_angle(drv))
#define ENCODER_GET_ANGLE(drv) ((drv)->ops->get_angle(drv))
#define ENCODER_GET_ACG(drv) ((drv)->ops->get_acg(drv))
#define ENCODER_GET_STATUS(drv) ((drv)->ops->get_status(drv))

#endif /* BSP_DRIVERS_INTERFACE_DEVICE_ABSOLUTE_ENCODER_DRIVER_H_ */
