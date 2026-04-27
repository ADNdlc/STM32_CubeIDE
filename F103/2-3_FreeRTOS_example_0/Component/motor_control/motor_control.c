#include "motor_control.h"
#include "sys.h"
#include <math.h>
#include <stdlib.h>
#include "elog.h"

// #include "MemPool.h"
#ifdef USE_MEMPOOL
#define MOTOR_CTRL_MEMSOURCE SYS_MEM_INTERNAL
#endif

// 手动定义PI常量
#define M_PI_F 3.14159265358979323846f
#define _3PI_2 4.71238898038f

#define _constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// 归一化角度到 [0,2PI]
float _normalizeAngle(float angle)
{
  float _2PI = 2.0f * M_PI_F;
  if (angle >= 0)
  {
    while (angle >= _2PI)
      angle -= _2PI;
  }
  else
  {
    while (angle < 0)
      angle += _2PI;
  }
  return angle;
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
  float Ua = Ualpha;
  float Ub = (sqrt(3) * Ubeta - Ualpha) / 2;
  float Uc = (-Ualpha - sqrt(3) * Ubeta) / 2;

  motor_set_phase_voltage(motor_ctrl->motor, Ua, Ub, Uc);
}

/**
 * @brief 根据机械角度获取对应电角度
 * @param self motor_t 实例
 * @param shaft_angle 机械角度
 * @return 电机电角度
 */
float get_electricalAngle_manual(motor_control_t *self, float shaft_angle)
{
  if (!self)
    return -1;
  return (self->motor->pole_pairs * shaft_angle);
}

float get_electricalAngle(motor_control_t *self)
{
  if (!self || !self->encoder)
    return -1;
  // 计算方向系数：true表示正向(+1)，false表示反向(-1)
  float direction_factor = self->direction ? 1.0f : -1.0f;
  return (self->motor->pole_pairs * direction_factor * ENCODER_GET_ANGLE(self->encoder) - self->zero_electric_angle);
}

/**
 * @brief 电机零点校准
 * @param self motor_t 实例
 * @return 0 成功，-1 失败
 * @note 
 * - 施加已知电角度电压(_3PI_2)使电机稳定在该位置
 * - 多次读取编码器值，判断电机是否已经稳定（连续多次读数变化小于阈值）
 * - 计算零点偏移量：zero_offset = known_electrical_angle - (pole_pairs * direction * mechanical_angle)
 * - 结果经过归一化处理确保在有效范围内
 */
int calibrate_zero_electric_angle(motor_control_t *self)
{
  if (!self || !self->encoder)
    return -1;
  
  // 1. 施加已知电角度电压，让电机转到 _3PI_2 位置（270度）
  setPhaseVoltage(self, 3, 0, _3PI_2);
  
  // 2. 等待初始稳定时间
  sys_delay_ms(500);
  
  // 3. 多次读取编码器值，判断电机是否稳定
  #define STABILITY_CHECK_COUNT 5    // 稳定性检查次数
  #define ANGLE_STABILITY_THRESHOLD 0.005f  // 角度稳定性阈值（弧度，约0.57度）
  
  float last_angle = ENCODER_GET_ANGLE(self->encoder);
  uint8_t stable_count = 0;
  uint32_t timeout_count = 0;
  const uint32_t MAX_TIMEOUT = 20; // 最大超时次数（每次100ms，总共2秒）
  
  while (stable_count < STABILITY_CHECK_COUNT && timeout_count < MAX_TIMEOUT)
  {
    sys_delay_ms(100); // 每次等待100ms
    
    float current_angle = ENCODER_GET_ANGLE(self->encoder);
    float angle_diff = fabsf(current_angle - last_angle);
    
    // 归一化角度差到 [0, π] 范围（处理角度环绕问题）
    if (angle_diff > M_PI_F){
      angle_diff = 2.0f * M_PI_F - angle_diff;
    }
    
    if (angle_diff < ANGLE_STABILITY_THRESHOLD){
      stable_count++;
    }
    else{
      stable_count = 0; // 重置稳定计数器
    }
    
    last_angle = current_angle;
    timeout_count++;
  }
  
  // 4. 检查是否成功稳定
  if (stable_count < STABILITY_CHECK_COUNT){
    // 超时未稳定，使用最后一次读取的角度
    log_w("Motor calibration timeout, using last angle reading");
  }
  
  // 5. 使用最终稳定的角度值
  float actual_mechanical_angle = last_angle;
  
  // 6. 计算方向系数：true表示正向(+1)，false表示反向(-1)
  float direction_factor = self->direction ? 1.0f : -1.0f;
  
  // 7. 计算理论电角度（不包含零点偏移）
  float theoretical_electrical_angle = self->motor->pole_pairs * direction_factor * actual_mechanical_angle;
  
  // 8. 计算零点偏移量：已知目标电角度 - 实际测量的理论电角度
  float zero_offset = _3PI_2 - theoretical_electrical_angle;
  
  // 9. 归一化零点偏移到 [0, 2π) 范围
  self->zero_electric_angle = _normalizeAngle(zero_offset);
  
  // 10. 停止电机
  setPhaseVoltage(self, 0, 0, _3PI_2);
  
  return 0;
}

motor_control_t *motor_control_create(threephase_motor_t *motor, absolute_encoder_driver_t *encoder, uint8_t pole_pairs)
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
  motor_ctrl->motor->pole_pairs = pole_pairs;
  motor_ctrl->shaft_angle = 0.0f;
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
