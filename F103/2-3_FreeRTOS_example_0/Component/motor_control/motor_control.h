#ifndef BSP_DEVICE_DRIVER_MOTOR_CONTROL_H_
#define BSP_DEVICE_DRIVER_MOTOR_CONTROL_H_

/** 
 * @brief 启用三相波形输出
 */
#define ENABLE_WAVEFORM_OUTPUT 1
#define BUF_SIZE 512 

#include "..\ThreePhase_motor\ThreePhase_motor.h"
#include <stdint.h>
#if ENABLE_WAVEFORM_OUTPUT
#include "utils\ring_buffer\ring_buffer.h"
#endif
typedef struct motor_control_t motor_control_t;


struct motor_control_t
{
  threephase_motor_t *motor;
  float shaft_angle;         // 机械角度
  float zero_electric_angle; // 电角度零点
  uint32_t loop_timestamp;
  // 波形数据缓冲
#if ENABLE_WAVEFORM_OUTPUT
  ring_buffer_t waveform_data;
  uint8_t *buf[BUF_SIZE];
#endif
};

motor_control_t *motor_control_create(threephase_motor_t *motor, uint8_t pole_pairs);
void motor_control_destroy(motor_control_t *self);
void setPhaseVoltage(motor_control_t *motor_ctrl, float Uq, float Ud, float angle_el);

#if ENABLE_WAVEFORM_OUTPUT
/**
 * @brief 获取波形数据
 * 
 * @param motor_ctrl 控制对象实例
 * @param buf 读取缓冲区
 * @param len 读取长度(每个长度为 3*float,即一帧三通道的数据)
 * @return int 0: 成功, -1: 失败
 */
int get_waveform_data(motor_control_t *motor_ctrl, float* buf, uint16_t len);
#endif

#endif /* BSP_DEVICE_DRIVER_MOTOR_CONTROL_H_ */
