#include "user_task.h"
#include "usart.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "cmsis_os.h"

#include "sys.h"
#include "motor_control/motor_control.h"
#include "BSP_init.h" // 包含以获取Motor_1_control和g_debug_queue的外部声明
#include "uart_queue/uart_queue.h" // 包含uart_queue相关函数

// 手动定义PI常量（与motor_control.c保持一致）
#define M_PI_F 3.14159265358979323846f

// 全局目标速度变量，可通过串口命令动态调整
static float target_velocity = 5.0f; // 默认目标速度 5 rad/s

// 归一化角度到 [0,2PI]
static float _normalizeAngle(float angle)
{
  float a = fmod(angle, 2 * M_PI_F);
  return a >= 0 ? a : (a + 2 * M_PI_F);
}

// 开环速度函数
float velocityOpenloop(float target_velocity)
{
    uint32_t now_us = sys_get_systick_us();

    // 计算当前每个Loop的运行时间间隔
    uint32_t Ts = now_us - Motor_1_control->loop_timestamp;

    // 通过乘以时间间隔和目标速度来计算需要转动的机械角度，存储在 shaft_angle 变量中。在此之前，还需要对轴角度进行归一化，以确保其值在 0 到 2π 之间。
    Motor_1_control->shaft_angle = _normalizeAngle(Motor_1_control->shaft_angle + target_velocity * Ts * 1e-6f); // Ts是微秒，转换为秒
    // 以目标速度为 10 rad/s 为例，如果时间间隔是 1 秒，则在每个循环中需要增加 10 * 1 = 10 弧度的角度变化量，才能使电机转动到目标速度。
    // 如果时间间隔是 0.1 秒，那么在每个循环中需要增加的角度变化量就是 10 * 0.1 = 1 弧度，才能实现相同的目标速度。因此，电机轴的转动角度取决于目标速度和时间间隔的乘积。

    // 使用早前设置的voltage_power_supply的1/3作为Uq值，这个值会直接影响输出力矩
    // 最大只能设置为Uq = voltage_power_supply/2，否则ua,ub,uc会超出供电电压限幅
    float Uq = Motor_1_control->motor->voltage_limit / 3;

    // 使用新添加的motor_get_electricalAngle函数获取电角度
    float electrical_angle = motor_get_electricalAngle(Motor_1_control->motor, Motor_1_control->shaft_angle);
    setPhaseVoltage(Motor_1_control, Uq, 0, electrical_angle);

    Motor_1_control->loop_timestamp = now_us; // 用于计算下一个时间间隔
    return Uq;
}

// 处理串口输入命令，动态设置目标速度
void User_Task_HandleInput(uint8_t cmd)
{
    // 忽略回车符和换行符
    if (cmd == '\r' || cmd == '\n') {
        return;
    }
    
    // 处理速度设置命令
    if (cmd >= '0' && cmd <= '9') {
        // 将字符转换为速度值 (0-9 rad/s)
        float new_velocity = (float)(cmd - '0');
        target_velocity = new_velocity;
        printf("Target velocity set to: %.1f rad/s\r\n", target_velocity);
    } else if (cmd == '+') {
        // 增加速度
        target_velocity += 1.0f;
        if (target_velocity > 20.0f) target_velocity = 20.0f; // 限制最大速度
        printf("Target velocity increased to: %.1f rad/s\r\n", target_velocity);
    } else if (cmd == '-') {
        // 减少速度
        target_velocity -= 1.0f;
        if (target_velocity < 0.0f) target_velocity = 0.0f; // 限制最小速度
        printf("Target velocity decreased to: %.1f rad/s\r\n", target_velocity);
    } else if (cmd == 's') {
        // 停止电机
        target_velocity = 0.0f;
        motor_stop(Motor_1_control->motor);
        printf("Motor stopped\r\n");
    } else if (cmd == 'm') {
        // 显示菜单
        printf("========================================\r\n");
        printf("       Motor Control Menu               \r\n");
        printf("========================================\r\n");
        printf("[0-9] Set speed (0-9 rad/s)\r\n");
        printf("[+]   Increase speed by 1 rad/s\r\n");
        printf("[-]   Decrease speed by 1 rad/s\r\n");
        printf("[s]   Stop motor\r\n");
        printf("[m]   Show this menu\r\n");
        printf("========================================\r\n");
        printf("Current target velocity: %.1f rad/s\r\n", target_velocity);
    } else {
        printf("Unknown command: '%c' (0x%02X)\r\n", cmd, cmd);
    }
}

void User_Task_1(void)
{
    // 检查串口输入并处理
    if (g_debug_queue) {
        uint8_t rx_char;
        while (uart_queue_getdata(g_debug_queue, &rx_char, 1) > 0) {
            User_Task_HandleInput(rx_char);
        }
    }
    
    // 执行开环速度控制
    velocityOpenloop(target_velocity);
}

void User_Task_2(void)
{
    vTaskDelay(500);
}

int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 10);
    return ch;
}