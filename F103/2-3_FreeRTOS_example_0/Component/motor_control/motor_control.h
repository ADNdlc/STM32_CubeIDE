#ifndef BSP_DEVICE_DRIVER_MOTOR_CONTROL_H_
#define BSP_DEVICE_DRIVER_MOTOR_CONTROL_H_


#include "..\ThreePhase_motor\ThreePhase_motor.h"
#include "Device\absolute_encoder_driver.h"
#include <stdint.h>

typedef struct motor_control_t motor_control_t;

struct motor_control_t{
  threephase_motor_t *motor;
  absolute_encoder_driver_t *encoder;
  bool direction; // 编码器方向
  float shaft_angle;         // 机械角度
  float zero_electric_angle; // 电角度零点
  uint32_t loop_timestamp;
};

motor_control_t *motor_control_create(threephase_motor_t *motor, absolute_encoder_driver_t *encoder, uint8_t pole_pairs);
void motor_control_destroy(motor_control_t *self);
void setPhaseVoltage(motor_control_t *motor_ctrl, float Uq, float Ud, float angle_el);

float get_electricalAngle_manual(motor_control_t *self, float shaft_angle);
float get_electricalAngle(motor_control_t *self, float shaft_angle);

int calibrate_zero_electric_angle(motor_control_t *self);
int motor_control_init(motor_control_t *self);

#endif /* BSP_DEVICE_DRIVER_MOTOR_CONTROL_H_ */
