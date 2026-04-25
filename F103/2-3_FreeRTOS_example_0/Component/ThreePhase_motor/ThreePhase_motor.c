#include "ThreePhase_motor.h"
#include "sys.h"
#include <stdint.h>
#include <stdlib.h>
// #include "MemPool.h"
#ifdef USE_MEMPOOL
#define TP_MOTOR_MEMSOURCE SYS_MEM_INTERNAL
#endif

#define _constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

void motor_init(threephase_motor_t *self, pwm_driver_t *phase_u, pwm_driver_t *phase_v, pwm_driver_t *phase_w, float bus_voltage)
{
  if (!self || !phase_u || !phase_v || !phase_w)
    return;
  self->phase_u = phase_u;
  self->phase_v = phase_v;
  self->phase_w = phase_w;
  self->bus_voltage = bus_voltage;
  self->voltage_limit = 0.5f * bus_voltage;
}

threephase_motor_t *threephase_motor_create(pwm_driver_t *phase_u, pwm_driver_t *phase_v, pwm_driver_t *phase_w, float bus_voltage)
{
  if (!phase_u || !phase_v || !phase_w)
    return NULL;
#ifdef USE_MEMPOOL
  threephase_motor_t *motor =
      (threephase_motor_t *)sys_malloc(TP_MOTOR_MEMSOURCE, sizeof(threephase_motor_t));
#else
  threephase_motor_t *motor =
      (threephase_motor_t *)malloc(sizeof(threephase_motor_t));
#endif
  if (!motor)
  {
    return NULL;
  }
  PWM_STOP(phase_u);
  PWM_STOP(phase_v);
  PWM_STOP(phase_w);

  motor_init(motor, phase_u, phase_v, phase_w, bus_voltage);
  uint32_t max_duty_u = PWM_GET_DUTY_MAX(motor->phase_u);
  uint32_t max_duty_v = PWM_GET_DUTY_MAX(motor->phase_v);
  uint32_t max_duty_w = PWM_GET_DUTY_MAX(motor->phase_w);
  if (!(max_duty_u == max_duty_v && max_duty_u == max_duty_w))
  {
    motor_destroy(motor);
    return NULL;
  }

  return motor;
}

void motor_destroy(threephase_motor_t *self)
{
  if (self)
  {
    // 停止PWM
    PWM_STOP(self->phase_u);
    PWM_STOP(self->phase_v);
    PWM_STOP(self->phase_w);

#ifdef USE_MEMPOOL
    sys_free(TP_MOTOR_MEMSOURCE, self);
#else
    free(self);
#endif
  }
}

int motor_set_phase_voltage(threephase_motor_t *self, float Vu, float Vv, float Vw)
{
  if (!self)
    return -1;

  float dc_u = _constrain(Vu / self->voltage_limit, 0.0f, 1.0f);
  float dc_v = _constrain(Vv / self->voltage_limit, 0.0f, 1.0f);
  float dc_w = _constrain(Vw / self->voltage_limit, 0.0f, 1.0f);

  uint32_t max_duty = PWM_GET_DUTY_MAX(self->phase_u);

  PWM_SET_DUTY(self->phase_u, max_duty * dc_u);
  PWM_SET_DUTY(self->phase_v, max_duty * dc_v);
  PWM_SET_DUTY(self->phase_w, max_duty * dc_w);

  PWM_START(self->phase_u);
  PWM_START(self->phase_v);
  PWM_START(self->phase_w);
  return 0;
}

float motor_get_electricalAngle(threephase_motor_t *self, float shaft_angle)
{
  if (!self)
    return -1;
  return (shaft_angle * self->pole_pairs);
}

void motor_set_voltage(threephase_motor_t *self, float bus_voltage, float voltage_limit)
{
  if (!self)
    return;
  motor_set_phase_voltage(self, 0.0f, 0.0f, 0.0f);
  self->bus_voltage = bus_voltage;
  self->voltage_limit = voltage_limit;
}

void motor_get_bus_voltage(threephase_motor_t *self, float *bus_voltage, float *voltage_limit)
{
  if (!self)
    return;
  *bus_voltage = (self->bus_voltage);
  *voltage_limit = (self->voltage_limit);
}

void motor_stop(threephase_motor_t *self)
{
  if (!self)
    return;
  PWM_STOP(self->phase_u);
  PWM_STOP(self->phase_v);
  PWM_STOP(self->phase_w);
}
