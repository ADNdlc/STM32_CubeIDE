#include "motor_control.h"
#include "sys.h"
#include <math.h>
#include <stdlib.h>
// #include "MemPool.h"
#ifdef USE_MEMPOOL
#define TP_MOTOR_MEMSOURCE SYS_MEM_INTERNAL
#endif

#define _constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// 归一化角度到 [0,2PI]
float _normalizeAngle(float angle)
{
  float a = fmod(angle, 2 * M_PI); // 取余运算可以用于归一化
  return a >= 0 ? a : (a + 2 * M_PI);
  // fmod 函数的余数的符号与除数相同。因此，当 angle 的值为负数时，余数的符号将与 _2PI 的符号相反。也就是说，如果 angle 的值小于 0 且 _2PI 的值为正数，则 fmod(angle, _2PI) 的余数将为负数。
  // 例如，当 angle 的值为 -PI/2，_2PI 的值为 2PI 时，fmod(angle, _2PI) 将返回一个负数。在这种情况下，可以通过将负数的余数加上 _2PI 来将角度归一化到 [0, 2PI] 的范围内，以确保角度的值始终为正数。
}

/**
 * @brief 
 * 
 * @param motor_ctrl 
 * @param Uq 
 * @param Ud 
 * @param angle_el 
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

threephase_motor_t *pwm_led_create(threephase_motor_t *self, pwm_driver_t *phase_u, pwm_driver_t *phase_v, pwm_driver_t *phase_w, float bus_voltage)
{
  if (!self)
    return NULL;
#ifdef USE_MEMPOOL
  threephase_motor_t *motor =
      (threephase_motor_t *)sys_malloc(TP_MOTOR_MEMSOURCE, sizeof(threephase_motor_t));
#else
  threephase_motor_t *motor =
      (threephase_motor_t *)malloc(sizeof(threephase_motor_t));
#endif
}

void motor_destroy(threephase_motor_t *self)
{
  if (self)
  {

#ifdef USE_MEMPOOL
    sys_free(PWMLED_MEMSOURCE, self);
#else
    free(self);
#endif
  }
}
