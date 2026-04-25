#include "user_task.h"
#include "usart.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "cmsis_os.h"

#include "sys.h"
#include "motor_control\motor_control.h"
#include "BSP_init.h"

// 开环速度函数
float velocityOpenloop(float target_velocity)
{
    uint32_t now_us = sys_get_systick_us();

    // 计算当前每个Loop的运行时间间隔
    uint32_t Ts = now_us - Motor_1_control->loop_timestamp;

    // 通过乘以时间间隔和目标速度来计算需要转动的机械角度，存储在 shaft_angle 变量中。在此之前，还需要对轴角度进行归一化，以确保其值在 0 到 2π 之间。
    Motor_1_control->shaft_angle = _normalizeAngle(Motor_1_control->shaft_angle + target_velocity * Ts);
    // 以目标速度为 10 rad/s 为例，如果时间间隔是 1 秒，则在每个循环中需要增加 10 * 1 = 10 弧度的角度变化量，才能使电机转动到目标速度。
    // 如果时间间隔是 0.1 秒，那么在每个循环中需要增加的角度变化量就是 10 * 0.1 = 1 弧度，才能实现相同的目标速度。因此，电机轴的转动角度取决于目标速度和时间间隔的乘积。

    // 使用早前设置的voltage_power_supply的1/3作为Uq值，这个值会直接影响输出力矩
    // 最大只能设置为Uq = voltage_power_supply/2，否则ua,ub,uc会超出供电电压限幅
    float Uq = Motor_1_control->motor->voltage_limit / 3;

    setPhaseVoltage(Motor_1_control, Uq, 0, motor_get_electricalAngle(Motor_1_control->motor, Motor_1_control->shaft_angle));

    Motor_1_control->loop_timestamp = now_us; // 用于计算下一个时间间隔
    return Uq;
}

void User_Task_1(void)
{
    velocityOpenloop(5);
}

void User_Task_2(void)
{
    vTaskDelay(500);
}

int fputc(int ch, FILE *f)
{
    // HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 10);
    while (!(huart1.Instance->SR & USART_SR_TXE))
        ;
    huart1.Instance->DR = *(uint8_t *)&ch;
    return ch;
}
