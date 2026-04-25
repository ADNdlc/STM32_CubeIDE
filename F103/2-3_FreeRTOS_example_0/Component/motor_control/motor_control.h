#ifndef BSP_DEVICE_DRIVER_MOTOR_CONTROL_H_
#define BSP_DEVICE_DRIVER_MOTOR_CONTROL_H_

#include "ThreePhase_motor.h"
#include <stdint.h>

typedef struct motor_control_t motor_control_t;

struct motor_control_t
{
  threephase_motor_t * motor;
  uint8_t pole_pairs; // 极对数
  float shaft_angle;  // 机械角度
  float electric_angle;// 电角度
  float zero_electric_angle;// 电角度零点
  uint32_t loop_timestamp;
};



#endif /* BSP_DEVICE_DRIVER_MOTOR_CONTROL_H_ */
