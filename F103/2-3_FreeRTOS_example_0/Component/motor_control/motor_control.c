#include "motor_control.h"
#include "sys.h"
#include <math.h>
#include <stdlib.h>
// #include "MemPool.h"
#ifdef USE_MEMPOOL
#define MOTOR_CTRL_MEMSOURCE SYS_MEM_INTERNAL
#endif

// 手动定义PI常量
#define M_PI_F 3.14159265358979323846f

#define _constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// 归一化角度到 [0,2PI]
float _normalizeAngle(float angle)
{
  float a = fmod(angle, 2 * M_PI_F); // 取余运算可以用于归一化
  return a >= 0 ? a : (a + 2 * M_PI_F);
  // fmod 函数的余数的符号与除数相同。因此，当 angle 的值为负数时，余数的符号将与 _2PI 的符号相反。也就是说，如果 angle 的值小于 0 且 _2PI 的值为正数，则 fmod(angle, _2PI) 的余数将为负数。
  // 例如，当 angle 的值为 -PI/2，_2PI 的值为 2PI 时，fmod(angle, _2PI) 将返回一个负数。在这种情况下，可以通过将负数的余数加上 _2PI 来将角度归一化到 [0, 2PI] 的范围内，以确保角度的值始终为正数。
}

/**
 * @brief 设置电机相电压（基于FOC控制）
 *
 * @param motor_ctrl 电机控制实例指针
 * @param Uq d轴电压分量（通常用于控制转矩，Ud通常设为0用于最大转矩控制）
 * @param Ud q轴电压分量（通常设为0，用于弱磁控制时可调整）
 * @param angle_el 电角度（弧度），范围[0, 2π]
 *
 * @note
 * - 电角度会自动加上零点偏移量（zero_electric_angle）进行校正
 * - 最终输出的三相电压会自动加上母线电压的一半作为偏置，确保PWM占空比在有效范围内
 */
void setPhaseVoltage(motor_control_t *motor_ctrl, float Uq, float Ud, float angle_el)
{
  angle_el = _normalizeAngle(angle_el + motor_ctrl->zero_electric_angle);
  // 帕克逆变换(忽略 Ud)
  float Ualpha = -Uq * sin(angle_el);
  float Ubeta = Uq * cos(angle_el);

  // 克拉克逆变换
  float Ua = Ualpha + motor_ctrl->motor->voltage_limit / 2;
  float Ub = (sqrt(3) * Ubeta - Ualpha) / 2 + motor_ctrl->motor->voltage_limit / 2;
  float Uc = (-Ualpha - sqrt(3) * Ubeta) / 2 + motor_ctrl->motor->voltage_limit / 2;

  motor_set_phase_voltage(motor_ctrl->motor, Ua, Ub, Uc);
}

motor_control_t *motor_control_create(threephase_motor_t *motor, uint8_t pole_pairs)
{
  if (!motor)
    return NULL;

#ifdef USE_MEMPOOL
  motor_control_t *motor_ctrl =
      (motor_control_t *)sys_malloc(MOTOR_CTRL_MEMSOURCE, sizeof(motor_control_t));
#else
  motor_control_t *motor_ctrl =
      (motor_control_t *)malloc(sizeof(motor_control_t));
#endif

  if (!motor_ctrl)
    return NULL;

  motor_ctrl->motor = motor;
  motor_ctrl->pole_pairs = pole_pairs;
  motor_ctrl->shaft_angle = 0.0f;
  motor_ctrl->electric_angle = 0.0f;
  motor_ctrl->zero_electric_angle = 0.0f;
  motor_ctrl->loop_timestamp = 0;

  return motor_ctrl;
}

void motor_control_destroy(motor_control_t *self)
{
  if (self)
  {
#ifdef USE_MEMPOOL
    sys_free(MOTOR_CTRL_MEMSOURCE, self);
#else
    free(self);
#endif
  }
}