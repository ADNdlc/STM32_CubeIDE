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
  // angle_el 应该是转子的实时电角度（由 get_electricalAngle 获取）
  angle_el = _normalizeAngle(angle_el);
  
  // 帕克逆变换 (考虑 Ud 和 Uq)
  // 为了产生转矩，定子磁场需领先转子磁场 90 度
  float Ualpha = -Uq * sin(angle_el) + Ud * cos(angle_el);
  float Ubeta =  Uq * cos(angle_el) + Ud * sin(angle_el);

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

float get_electricalAngle(motor_control_t *self, float shaft_angle)
{
  if (!self)
    return -1;
  // 计算方向系数：true表示正向(+1)，false表示反向(-1)
  float direction_factor = self->direction ? 1.0f : -1.0f;
  float raw_elec_angle = self->motor->pole_pairs * direction_factor * shaft_angle;
  return _normalizeAngle(raw_elec_angle + self->zero_electric_angle);
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
  
  // 1. 施加电压使电机对齐到电角度 0 
  // 由于 setPhaseVoltage 内部会加 90度，所以我们要传入 -90度 (_3PI_2) 来产生 0度的磁场
  setPhaseVoltage(self, 3, 0, _3PI_2);
  
  // 2. 等待稳定
  sys_delay_ms(700);
  
  // 3. 多次读取并检查稳定性
  #define STABILITY_CHECK_COUNT 5
  #define ANGLE_STABILITY_THRESHOLD 0.005f
  
  float last_angle = ENCODER_GET_ANGLE(self->encoder);
  uint8_t stable_count = 0;
  uint32_t timeout_count = 0;
  
  while (stable_count < STABILITY_CHECK_COUNT && timeout_count < 20)
  {
    sys_delay_ms(100);
    float current_angle = ENCODER_GET_ANGLE(self->encoder);
    float angle_diff = fabsf(current_angle - last_angle);
    if (angle_diff > M_PI_F) angle_diff = 2.0f * M_PI_F - angle_diff;
    
    if (angle_diff < ANGLE_STABILITY_THRESHOLD) stable_count++;
    else stable_count = 0;
    
    last_angle = current_angle;
    timeout_count++;
  }

  // 4. 计算零点偏移量
  // 此时电机物理位置在电角度 0，所以我们希望 get_electricalAngle 返回 0
  // 公式：0 = (pole_pairs * direction * mechanical_angle) + zero_offset
  // 所以：zero_offset = -(pole_pairs * direction * mechanical_angle)
  float direction_factor = self->direction ? 1.0f : -1.0f;
  float raw_elec_angle = self->motor->pole_pairs * direction_factor * last_angle;
  
  self->zero_electric_angle = _normalizeAngle(-raw_elec_angle);
  
  // 5. 停止电机
  setPhaseVoltage(self, 0, 0, 0);
  
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
  motor_ctrl->encoder = encoder; // 赋值编码器
  motor_ctrl->motor->pole_pairs = pole_pairs;
  motor_ctrl->shaft_angle = 0.0f;
  motor_ctrl->zero_electric_angle = 0.0f;
  motor_ctrl->direction = true;   // 默认正向
  motor_ctrl->loop_timestamp = 0;

  return motor_ctrl;
}

/**
 * @brief 电机控制初始化（自动辨识方向 + 校准零点）
 */
int motor_control_init(motor_control_t *self)
{
  if (!self || !self->encoder) return -1;
  
  log_i("Starting motor initialization...");

  // 1. 自动辨识方向
  // 移动到电角度 0
  setPhaseVoltage(self, 3, 0, 0);
  sys_delay_ms(500);
  float angle_start = ENCODER_GET_ANGLE(self->encoder);
  
  // 移动到电角度 PI/2 (90度)
  setPhaseVoltage(self, 3, 0, M_PI_F / 2.0f);
  sys_delay_ms(500);
  float angle_end = ENCODER_GET_ANGLE(self->encoder);
  
  // 计算位移并处理过零点
  float diff = angle_end - angle_start;
  if (diff > M_PI_F) diff -= 2.0f * M_PI_F;
  if (diff < -M_PI_F) diff += 2.0f * M_PI_F;
  
  if (diff > 0) {
    self->direction = true;
    log_i("Encoder direction: Normal");
  } else {
    self->direction = false;
    log_i("Encoder direction: Inverted");
  }

  // 2. 执行零点校准
  int ret = calibrate_zero_electric_angle(self);
  if (ret == 0) {
    log_i("Calibration success. Zero offset: %.2f rad", self->zero_electric_angle);
  } else {
    log_e("Calibration failed!");
  }
  
  return ret;
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
