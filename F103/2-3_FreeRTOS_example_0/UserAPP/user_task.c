#include "user_task.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "queue.h"
#include "semphr.h"
#include "stdio.h"
#include "usart.h"


#include "../A_Project_cfg/Project_cfg.h" // 包含项目配置
#include "BSP_init.h" // 包含以获取Motor_1_control和g_debug_queue的外部声明
#include "dev_map.h"
#include "elog.h"
#include "factory_inc.h"
#include "main.h" // 添加main.h以获取GPIO definition
#include "motor_control/motor_control.h"
#include "pwm_led/pwm_led.h"
#include "sys.h"
#include "uart_queue/uart_queue.h" // 包含uart_queue相关函数
#include "shell.h"
#include <math.h>


// 手动定义PI常量（与motor_control.c保持一致）
#define M_PI_F 3.14159265358979323846f

/* ----- 电机控制模式配置 ----- */
// 0: 开环速度控制模式
// 1: 位置闭环控制模式
#define POSITION_CONTROL_MODE 1

// 位置闭环控制模式下的全局目标位置变量
static float target_position = 0.0f; // 默认目标位置 0 弧度
// 开环速度控制模式下的全局目标速度变量
static float target_velocity = 5.0f; // 默认目标速度 5 rad/s

// 归一化角度到 [0,2PI]
static float _normalizeAngle(float angle) {
  float _2PI = 2.0f * M_PI_F;
  if (angle >= 0) {
    while (angle >= _2PI)
      angle -= _2PI;
  } else {
    while (angle < 0)
      angle += _2PI;
  }
  return angle;
}

#if !POSITION_CONTROL_MODE
// 开环速度函数
float velocityOpenloop(float target_velocity) {
  uint32_t now_us = sys_get_systick_us();
  static uint32_t last_timestamp = 0;
  static uint32_t last_timestamp_m2 = 0;

  if (last_timestamp == 0) last_timestamp = now_us - 1000;
  if (last_timestamp_m2 == 0) last_timestamp_m2 = now_us - 1000;

  uint32_t Ts = now_us - last_timestamp;
  uint32_t Ts_m2 = now_us - last_timestamp_m2;

  if (Ts == 0) Ts = 1000;
  if (Ts_m2 == 0) Ts_m2 = 1000;
  if (Ts > 100000) Ts = 1000;
  if (Ts_m2 > 100000) Ts_m2 = 1000;

  Motor_1_control->shaft_angle = _normalizeAngle(Motor_1_control->shaft_angle + target_velocity * (float)Ts * 1e-6f);
  float Uq = 1.5f + fabs(target_velocity) * 0.4f;
  if (Uq > Motor_1_control->motor->voltage_limit) Uq = Motor_1_control->motor->voltage_limit;
  
  float electrical_angle = get_electricalAngle_manual(Motor_1_control, Motor_1_control->shaft_angle);
  setPhaseVoltage(Motor_1_control, Uq, 0, electrical_angle);

  if (Motor_2_control) {
    Motor_2_control->shaft_angle = _normalizeAngle(Motor_2_control->shaft_angle + target_velocity * (float)Ts_m2 * 1e-6f);
    float Uq_m2 = 1.5f + fabs(target_velocity) * 0.4f;
    if (Uq_m2 > Motor_2_control->motor->voltage_limit) Uq_m2 = Motor_2_control->motor->voltage_limit;
    float electrical_angle_m2 = get_electricalAngle_manual(Motor_2_control, Motor_2_control->shaft_angle);
    setPhaseVoltage(Motor_2_control, Uq_m2, 0, electrical_angle_m2);
  }

  last_timestamp = now_us;
  last_timestamp_m2 = now_us;
  return Uq;
}
#endif

#define _constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

#if POSITION_CONTROL_MODE
/**
 * @brief 电机位置闭环控制函数
 */
void positioncloseloop(float target_position) {
  static const float Kp = 1.1f;
  static const float Ki = 0.1f; 
  static const float Kd = 0.05f; 

  static float integral_error = 0.0f;
  static float last_error = 0.0f;
  static uint32_t last_timestamp = 0;

  uint32_t current_timestamp = sys_get_systick_us();
  if (last_timestamp == 0) last_timestamp = current_timestamp - 1000;

  float dt = (current_timestamp - last_timestamp) * 1e-6f;
  if (dt <= 0 || dt > 0.1f) dt = 0.001f;

  // 1. 获取当前机械角度（弧度）- 控制循环核心读取
  float current_mech_angle = ENCODER_GET_ANGLE(g_encoder_m0);
  
  // 2. 根据电机方向调整反馈极性
  float feedback_pos = current_mech_angle;
  if (!Motor_1_control->direction) {
    feedback_pos = -feedback_pos;
  }

  // 3. 计算位置误差并处理环绕
  float error = target_position - feedback_pos;
  while (error > M_PI_F) error -= 2.0f * M_PI_F;
  while (error < -M_PI_F) error += 2.0f * M_PI_F;

  // 4. PID 计算
  integral_error += error * dt;
  float integral_max = Motor_1_control->motor->bus_voltage / Ki;
  integral_error = _constrain(integral_error, -integral_max, integral_max);
  float derivative_error = (error - last_error) / dt;
  float Uq = Kp * error + Ki * integral_error + Kd * derivative_error;

  // 5. 电压限制
  float voltage_limit = Motor_1_control->motor->bus_voltage;
  Uq = _constrain(Uq, -voltage_limit, voltage_limit);

  // 6. 获取电角度（复用读取到的机械角度）
  float electrical_angle = get_electricalAngle(Motor_1_control, current_mech_angle);

  // 7. 执行控制
  setPhaseVoltage(Motor_1_control, Uq, 0, electrical_angle);

  last_error = error;
  last_timestamp = current_timestamp;
}
#endif

void motor_set_pos(float pos) {
    HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_SET);
    target_position = pos;
    log_i("Target position set to: %.2f rad", target_position);
}

void motor_set_vel(float vel) {
    HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_SET);
    target_velocity = vel;
    log_i("Target velocity set to: %.1f rad/s", target_velocity);
}

void motor_stop_cmd(void) {
    HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_RESET);
    log_i("Motor disabled");
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), motor_pos, motor_set_pos, set motor target position);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), motor_vel, motor_set_vel, set motor target velocity);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), motor_stop, motor_stop_cmd, stop motor);

extern motor_control_t *Motor_1_control;
extern motor_control_t *Motor_2_control;
extern absolute_encoder_driver_t *g_encoder_m0;
extern absolute_encoder_driver_t *g_encoder_m1;

void User_Task_1(void) {
  // 串口输入现在由 Letter-Shell 统一处理

#if POSITION_CONTROL_MODE
  positioncloseloop(target_position);
#else
  velocityOpenloop(target_velocity);

  static uint32_t last_encoder_tick = 0;
  if (sys_get_systick_ms() - last_encoder_tick >= 50) {
    last_encoder_tick = sys_get_systick_ms();
    if (g_encoder_m0) {
      static float last_logged_angle_m0 = -10.0f;
      float current_angle = ENCODER_GET_ANGLE(g_encoder_m0);
      if (fabs(current_angle - last_logged_angle_m0) > 0.05f) {
        log_d("M0 Angle: %.2f rad", current_angle);
        last_logged_angle_m0 = current_angle;
      }
    }
  }
#endif
}

void User_Task_2(void) {
  static pwm_led_t *pwm_m0_in_1 = NULL;
  static uint32_t pwm_update_tick = 0;
  static uint32_t log_print_tick = 0;
  static int8_t direction = 1;
  static uint8_t brightness = 0;

  if (pwm_m0_in_1 == NULL) {
    pwm_driver_t *driver = pwm_driver_get(M0_IN_1);
    if (driver) pwm_m0_in_1 = pwm_led_create(1000, driver, 1);
  }

  if (sys_get_systick_ms() - pwm_update_tick >= 20) {
    pwm_update_tick = sys_get_systick_ms();
    if (pwm_m0_in_1) pwm_led_set_brightness(pwm_m0_in_1, brightness);
    brightness += direction;
    if (brightness >= 100 || brightness <= 0) direction = -direction;
  }
  vTaskDelay(1);
}

int fputc(int ch, FILE *f) {
  uint8_t data = (uint8_t)ch;
  if (g_debug_queue) uart_queue_send(g_debug_queue, &data, 1);
  else HAL_UART_Transmit(&huart1, &data, 1, 10);
  return ch;
}
