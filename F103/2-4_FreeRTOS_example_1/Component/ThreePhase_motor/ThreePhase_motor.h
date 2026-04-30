#ifndef BSP_DEVICE_DRIVER_THREEPHASE_MOTOR_H_
#define BSP_DEVICE_DRIVER_THREEPHASE_MOTOR_H_

#include "interface_inc.h"
#include <stdint.h>

typedef struct threephase_motor_t threephase_motor_t;

// Motor_t 结构体
struct threephase_motor_t
{
  pwm_driver_t *phase_u; // u相PWM通道
  pwm_driver_t *phase_v; // v相
  pwm_driver_t *phase_w; // w相

  float bus_voltage;   // 母线电压(电机驱动电压)
  uint8_t pole_pairs;  // 极对数
};

// 公共 API
void motor_init(threephase_motor_t *self, pwm_driver_t *phase_u, pwm_driver_t *phase_v, pwm_driver_t *phase_w, float bus_voltage);
threephase_motor_t *threephase_motor_create(pwm_driver_t *phase_u, pwm_driver_t *phase_v, pwm_driver_t *phase_w, float bus_voltage);
void motor_destroy(threephase_motor_t *self);

/**
 * @brief 设置三相相电压(静止坐标系，对电机中性点)
 * @param self motor_t 实例
 * @param Vu   U 相电压 (V)
 * @param Vv   V 相电压 (V)
 * @param Vw   W 相电压 (V)
 * @return 0: 成功, -1: 失败
 * @note  如果任一相电压超出单相(voltage_limit/2)限制, 将自动进行等比例缩放 (clamp)
 */
int motor_set_phase_voltage(threephase_motor_t *self, float Vu, float Vv, float Vw);

/**
 * @brief 停止电机
 */
void motor_stop(threephase_motor_t *self);

#endif /* BSP_DEVICE_DRIVER_THREEPHASE_MOTOR_H_ */
