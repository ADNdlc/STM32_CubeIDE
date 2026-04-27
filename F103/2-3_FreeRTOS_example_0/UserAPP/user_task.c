#include "user_task.h"
#include "usart.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "cmsis_os.h"

#include "sys.h"
#include "elog.h"
#include "motor_control/motor_control.h"
#include "BSP_init.h"              // 包含以获取Motor_1_control和g_debug_queue的外部声明
#include "uart_queue/uart_queue.h" // 包含uart_queue相关函数
#include "dev_map.h"
#include "factory_inc.h"
#include "pwm_led/pwm_led.h"
#include "main.h" // 添加main.h以获取GPIO定义
#include <math.h>
#include "../A_Project_cfg/Project_cfg.h" // 包含项目配置

// 手动定义PI常量（与motor_control.c保持一致）
#define M_PI_F 3.14159265358979323846f

/* ----- 电机控制模式配置 ----- */
// 0: 开环速度控制模式（默认）
// 1: 位置闭环控制模式
#define POSITION_CONTROL_MODE 0

#if POSITION_CONTROL_MODE
// 位置闭环控制模式下的全局目标位置变量
static float target_position = 0.0f; // 默认目标位置 0 弧度
#else
// 开环速度控制模式下的全局目标速度变量
static float target_velocity = 5.0f; // 默认目标速度 5 rad/s
#endif

// 归一化角度到 [0,2PI]
static float _normalizeAngle(float angle)
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

#if !POSITION_CONTROL_MODE
// 开环速度函数（仅在开环模式下编译）
float velocityOpenloop(float target_velocity)
{
    uint32_t now_us = sys_get_systick_us();
    static uint32_t last_timestamp = 0;
    static uint32_t last_timestamp_m2 = 0;

    if (last_timestamp == 0)
        last_timestamp = now_us - 1000;
    if (last_timestamp_m2 == 0)
        last_timestamp_m2 = now_us - 1000;

    // 计算当前每个Loop的运行时间间隔
    uint32_t Ts = now_us - last_timestamp;
    uint32_t Ts_m2 = now_us - last_timestamp_m2;

    // 如果微秒计时器失效(Ts=0)，则强制使用1ms步长，确保电机能转动
    if (Ts == 0)
        Ts = 1000;
    if (Ts_m2 == 0)
        Ts_m2 = 1000;
    // 过滤异常大的 Ts (例如断点调试后恢复)
    if (Ts > 100000)
        Ts = 1000;
    if (Ts_m2 > 100000)
        Ts_m2 = 1000;

    // 累加机械角度 - Motor 1
    Motor_1_control->shaft_angle = _normalizeAngle(Motor_1_control->shaft_angle + target_velocity * (float)Ts * 1e-6f);
    // 使用 V/f 控制：电压随速度增加，以克服反电动势
    // 基础电压 1.5V + 每 rad/s 增加 0.4V
    float Uq = 1.5f + fabs(target_velocity) * 0.4f;
    // 限制在可用电压范围内 - Motor 1
    if (Uq > Motor_1_control->motor->voltage_limit)
    {
        Uq = Motor_1_control->motor->voltage_limit;
    }
    // 获取电角度并设置电压 - Motor 1
    float electrical_angle = get_electricalAngle_manual(Motor_1_control, Motor_1_control->shaft_angle);
    setPhaseVoltage(Motor_1_control, Uq, 0, electrical_angle);

    // 处理 Motor 2
    if (Motor_2_control)
    {
        // 累加机械角度 - Motor 2
        Motor_2_control->shaft_angle = _normalizeAngle(Motor_2_control->shaft_angle + target_velocity * (float)Ts_m2 * 1e-6f);

        // 使用相同的 V/f 控制策略
        float Uq_m2 = 1.5f + fabs(target_velocity) * 0.4f;
        if (Uq_m2 > Motor_2_control->motor->voltage_limit)
        {
            Uq_m2 = Motor_2_control->motor->voltage_limit;
        }

        // 获取电角度并设置电压 - Motor 2
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
 * @param target_position 目标位置（弧度）
 * @note 
 * - 使用PID控制器实现精确位置控制
 * - 位置误差 = 目标位置 - 当前机械位置（已考虑方向）
 * - 输出电压Uq经过对称限幅处理
 * - 控制周期约为1ms（基于sys_get_systick_us调用频率）
 */
void positioncloseloop(float target_position)
{
    // PID控制器参数（可根据实际电机调整）
    static const float Kp = 0.8f;   // 比例增益
    static const float Ki = 0.05f;  // 积分增益  
    static const float Kd = 0.01f;  // 微分增益
    
    // 静态变量用于保存历史状态
    static float integral_error = 0.0f;
    static float last_error = 0.0f;
    static uint32_t last_timestamp = 0;
    
    // 获取当前时间戳（微秒）
    uint32_t current_timestamp = sys_get_systick_us();
    
    // 初始化时间戳
    if (last_timestamp == 0) {
        last_timestamp = current_timestamp - 1000; // 假设初始控制周期为1ms
    }
    
    // 计算控制周期（秒）
    float dt = (current_timestamp - last_timestamp) * 1e-6f;
    // 限制dt范围防止异常值
    if (dt <= 0 || dt > 0.1f) {
        dt = 0.001f; // 默认1ms
    }
    
    // 获取当前机械角度（弧度）
    float current_mechanical_angle = ENCODER_GET_ANGLE(g_encoder_m0);
    
    // 根据电机方向调整当前位置（确保与目标位置在同一坐标系）
    if (!Motor_1_control->direction) {
        current_mechanical_angle = -current_mechanical_angle;
    }
    
    // 计算位置误差（弧度）
    float error = target_position - current_mechanical_angle;
    
    // 归一化误差到 [-π, π] 范围（处理角度环绕问题）
    while (error > M_PI_F) {
        error -= 2.0f * M_PI_F;
    }
    while (error < -M_PI_F) {
        error += 2.0f * M_PI_F;
    }
    
    // PID计算
    // 积分项（带抗积分饱和）
    integral_error += error * dt;
    float integral_max = Motor_1_control->motor->voltage_limit / Ki;
    integral_error = _constrain(integral_error, -integral_max, integral_max);
    
    // 微分项
    float derivative_error = (error - last_error) / dt;
    
    // 计算输出电压
    float Uq = Kp * error + Ki * integral_error + Kd * derivative_error;
    
    // 对称电压限制
    float voltage_limit = Motor_1_control->motor->voltage_limit;
    Uq = _constrain(Uq, -voltage_limit, voltage_limit);
    
    // 获取当前电角度用于FOC控制
    float electrical_angle = get_electricalAngle(Motor_1_control);
    
    // 设置电机相电压
    setPhaseVoltage(Motor_1_control, Uq, 0, electrical_angle);
    
    // 更新历史状态
    last_error = error;
    last_timestamp = current_timestamp;
}
#endif

// 处理串口输入命令，动态设置目标速度或位置
void User_Task_HandleInput(uint8_t cmd)
{
    // 忽略回车符和换行符
    if (cmd == '\r' || cmd == '\n')
    {
        return;
    }

#if POSITION_CONTROL_MODE
    // 位置闭环控制模式
    if (cmd >= '0' && cmd <= '9')
    {
        HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_SET);
        // 将字符 '0'-'9' 映射到合理的位置范围 [0, 2π] 弧度
        // '0' -> 0 rad, '1' -> π/4 rad, '2' -> π/2 rad, ..., '8' -> 2π rad, '9' -> 0 rad (循环)
        int digit = cmd - '0';
        float position_range = 2.0f * M_PI_F; // 一圈的角度范围
        if (digit == 9) {
            target_position = 0.0f; // '9' 特殊处理为0位置
        } else {
            target_position = (float)digit * position_range / 8.0f;
        }
        log_i("Target position set to: %.2f rad (%.1f°)", target_position, target_position * 180.0f / M_PI_F);
    }
    else if (cmd == '+')
    {
        HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_SET);
        // 增加位置（每次增加 π/8 弧度，约22.5度）
        target_position += M_PI_F / 8.0f;
        // 归一化到 [0, 2π] 范围
        target_position = _normalizeAngle(target_position);
        log_i("Target position increased to: %.2f rad (%.1f°)", target_position, target_position * 180.0f / M_PI_F);
    }
    else if (cmd == '-')
    {
        // 减少位置（每次减少 π/8 弧度，约22.5度）
        target_position -= M_PI_F / 8.0f;
        // 归一化到 [0, 2π] 范围
        target_position = _normalizeAngle(target_position);
        log_i("Target position decreased to: %.2f rad (%.1f°)", target_position, target_position * 180.0f / M_PI_F);
    }
    else if (cmd == 's')
    {
        HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_RESET);
        // 停止电机（设置目标位置为当前位置）
        target_position = ENCODER_GET_ANGLE(g_encoder_m0);
        if (!Motor_1_control->direction) {
            target_position = -target_position;
        }
        target_position = _normalizeAngle(target_position);
        log_i("Motor stopped at position: %.2f rad", target_position);
    }
    else if (cmd == 'm')
    {
        // 显示菜单
        log_i("========================================");
        log_i("       Position Control Menu            ");
        log_i("========================================");
        log_i("[0-8] Set position (0-2π rad)");
        log_i("[9]   Set position to 0 rad");
        log_i("[+]   Increase position by π/8 rad");
        log_i("[-]   Decrease position by π/8 rad");
        log_i("[s]   Stop motor (hold current position)");
        log_i("[m]   Show this menu");
        log_i("========================================");
        log_i("Current target position: %.2f rad (%.1f°)", target_position, target_position * 180.0f / M_PI_F);
    }
    else if (cmd >= 32 && cmd <= 126)
    {
        log_i("Unknown command: '%c' (0x%02X)", cmd, cmd);
    }
#else
    // 开环速度控制模式（原有逻辑）
    if (cmd >= '0' && cmd <= '9')
    {
        HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_SET);
        // 将字符转换为速度值 (0-9 rad/s)
        float new_velocity = (float)(cmd - '0');
        target_velocity = new_velocity;
        log_i("Target velocity set to: %.1f rad/s", target_velocity);
    }
    else if (cmd == '+')
    {
        HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_SET);
        // 增加速度
        target_velocity += 1.0f;
        if (target_velocity > 20.0f)
            target_velocity = 20.0f; // 限制最大速度
        log_i("Target velocity increased to: %.1f rad/s", target_velocity);
    }
    else if (cmd == '-')
    {
        // 减少速度
        target_velocity -= 1.0f;
        if (target_velocity < 0.0f)
            target_velocity = 0.0f; // 限制最小速度
        log_i("Target velocity decreased to: %.1f rad/s", target_velocity);
    }
    else if (cmd == 's')
    {
        HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_RESET);
        // 停止电机
        target_velocity = 0.0f;
        motor_stop(Motor_1_control->motor);
        log_i("Motor stopped");
    }
    else if (cmd == 'm')
    {
        // 显示菜单
        log_i("========================================");
        log_i("       Motor Control Menu               ");
        log_i("========================================");
        log_i("[0-9] Set speed (0-9 rad/s)");
        log_i("[+]   Increase speed by 1 rad/s");
        log_i("[-]   Decrease speed by 1 rad/s");
        log_i("[s]   Stop motor");
        log_i("[m]   Show this menu");
        log_i("========================================");
        log_i("Current target velocity: %.1f rad/s", target_velocity);
    }
    else if (cmd >= 32 && cmd <= 126)
    {
        log_i("Unknown command: '%c' (0x%02X)", cmd, cmd);
    }
#endif
}

extern motor_control_t *Motor_1_control;
extern motor_control_t *Motor_2_control;
extern absolute_encoder_driver_t *g_encoder_m0;
extern absolute_encoder_driver_t *g_encoder_m1;

void User_Task_1(void)
{
    // 检查串口输入并处理
    if (g_debug_queue)
    {
        uint8_t rx_char;
        while (uart_queue_getdata(g_debug_queue, &rx_char, 1) > 0)
        {
            User_Task_HandleInput(rx_char);
        }
    }

#if POSITION_CONTROL_MODE
    // 位置闭环控制模式
    positioncloseloop(target_position);
#else
    // 开环速度控制模式
    // 执行双电机开环速度控制 (同步)
    velocityOpenloop(target_velocity); // 现在velocityOpenloop内部同时处理Motor_1_control和Motor_2_control
#endif

    // 降低编码器采样频率，避免 I2C 阻塞导致控制循环抖动
    static uint32_t last_encoder_tick = 0;
    if (sys_get_systick_ms() - last_encoder_tick >= 50)
    {
        last_encoder_tick = sys_get_systick_ms();

        if (g_encoder_m0)
        {
            static float last_logged_angle_m0 = -10.0f;
            float current_angle = ENCODER_GET_ANGLE(g_encoder_m0);
            if (fabs(current_angle - last_logged_angle_m0) > 0.05f)
            {
                log_d("M0 Angle: %.2f rad", current_angle);
                last_logged_angle_m0 = current_angle;
            }
        }

        if (g_encoder_m1)
        {
            static float last_logged_angle_m1 = -10.0f;
            float current_angle = ENCODER_GET_ANGLE(g_encoder_m1);
            if (fabs(current_angle - last_logged_angle_m1) > 0.05f)
            {
                log_d("M1 Angle: %.2f rad", current_angle);
                last_logged_angle_m1 = current_angle;
            }
        }
    }
}

void User_Task_2(void)
{
    static pwm_led_t *pwm_m0_in_1 = NULL;
    static uint32_t pwm_update_tick = 0; // 用于PWM更新的时间跟踪
    static uint32_t log_print_tick = 0;  // 用于日志打印的时间跟踪
    static int8_t direction = 1;
    static uint8_t brightness = 0;

    // 初始化 PWM LED（只执行一次）
    if (pwm_m0_in_1 == NULL)
    {
        pwm_driver_t *driver = pwm_driver_get(M0_IN_1);

        if (driver == NULL)
        {
            log_e("M0_IN_1 PWM driver not found");
            vTaskDelay(500);
            return;
        }

        // 创建 PWM LED，频率 1kHz，高电平有效
        pwm_m0_in_1 = pwm_led_create(1000, driver, 1);

        if (pwm_m0_in_1 == NULL)
        {
            log_e("Failed to create PWM LED for M0_IN_1");
            vTaskDelay(500);
            return;
        }

        log_i("M0_IN_1 PWM initialized at 1kHz");
    }

    // 每 500ms 打印一次亮度
    if (sys_get_systick_ms() - log_print_tick >= 500)
    {
        log_print_tick = sys_get_systick_ms();
        log_i("M0_IN_1 PWM brightness: %d", brightness);
    }

    // 每 20ms 更新一次亮度，实现呼吸灯效果
    if (sys_get_systick_ms() - pwm_update_tick >= 20)
    {
        pwm_update_tick = sys_get_systick_ms();

        pwm_led_set_brightness(pwm_m0_in_1, brightness);

        brightness += direction;
        if (brightness >= 100 || brightness <= 0)
        {
            direction = -direction;
        }
    }

    // 短暂延时，避免占用过多CPU
    vTaskDelay(1);
}

int fputc(int ch, FILE *f)
{
    uint8_t data = (uint8_t)ch;
    if (g_debug_queue)
    {
        // 使用异步队列发送，解决与 DMA 的冲突并防止阻塞任务
        uart_queue_send(g_debug_queue, &data, 1);
    }
    else
    {
        // 队列未就绪时降级使用阻塞发送
        HAL_UART_Transmit(&huart1, &data, 1, 10);
    }
    return ch;
}
