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
#include "BSP_init.h" // 包含以获取Motor_1_control和g_debug_queue的外部声明
#include "uart_queue/uart_queue.h" // 包含uart_queue相关函数
#include "dev_map.h"
#include "factory_inc.h"
#include "pwm_led/pwm_led.h"
#include <math.h>

// 手动定义PI常量（与motor_control.c保持一致）
#define M_PI_F 3.14159265358979323846f

// 全局目标速度变量，可通过串口命令动态调整
static float target_velocity = 5.0f; // 默认目标速度 5 rad/s

// 归一化角度到 [0,2PI]
static float _normalizeAngle(float angle)
{
  float _2PI = 2.0f * M_PI_F;
  if (angle >= 0) {
    while (angle >= _2PI) angle -= _2PI;
  } else {
    while (angle < 0) angle += _2PI;
  }
  return angle;
}

// 开环速度函数
float velocityOpenloop(float target_velocity)
{
    uint32_t now_us = sys_get_systick_us();
    static uint32_t last_timestamp = 0;
    
    if (last_timestamp == 0) last_timestamp = now_us - 1000;

    // 计算当前每个Loop的运行时间间隔
    uint32_t Ts = now_us - last_timestamp;
    
    // 保护：如果微秒计时器失效(Ts=0)，则强制使用1ms步长，确保电机能转动
    if (Ts == 0) Ts = 1000;
    // 过滤异常大的 Ts (例如断点调试后恢复)
    if (Ts > 100000) Ts = 1000; 

    // 累加机械角度
    Motor_1_control->shaft_angle = _normalizeAngle(Motor_1_control->shaft_angle + target_velocity * (float)Ts * 1e-6f);

    // 使用 V/f 控制：电压随速度增加，以克服反电动势
    // 基础电压 1.5V + 每 rad/s 增加 0.4V
    float Uq = 1.5f + fabs(target_velocity) * 0.4f;
    
    // 限制在可用电压范围内
    if (Uq > Motor_1_control->motor->voltage_limit) {
        Uq = Motor_1_control->motor->voltage_limit;
    }

    // 获取电角度并设置电压
    float electrical_angle = motor_get_electricalAngle(Motor_1_control->motor, Motor_1_control->shaft_angle);
    setPhaseVoltage(Motor_1_control, Uq, 0, electrical_angle);

    last_timestamp = now_us;
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
        HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_SET);
        // 将字符转换为速度值 (0-9 rad/s)
        float new_velocity = (float)(cmd - '0');
        target_velocity = new_velocity;
        log_i("Target velocity set to: %.1f rad/s", target_velocity);
    } else if (cmd == '+') {
        HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_SET);
        // 增加速度
        target_velocity += 1.0f;
        if (target_velocity > 20.0f) target_velocity = 20.0f; // 限制最大速度
        log_i("Target velocity increased to: %.1f rad/s", target_velocity);
    } else if (cmd == '-') {
        // 减少速度
        target_velocity -= 1.0f;
        if (target_velocity < 0.0f) target_velocity = 0.0f; // 限制最小速度
        log_i("Target velocity decreased to: %.1f rad/s", target_velocity);
    } else if (cmd == 's') {
        HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_RESET);
        // 停止电机
        target_velocity = 0.0f;
        motor_stop(Motor_1_control->motor);
        log_i("Motor stopped");
    } else if (cmd == 'm') {
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
    } else if (cmd >= 32 && cmd <= 126) {
        log_i("Unknown command: '%c' (0x%02X)", cmd, cmd);
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

    // 读取编码器角度并在变化时打印
    if (g_encoder_m0) {
        static float last_logged_angle = -10.0f;
        float current_angle = ENCODER_GET_ANGLE(g_encoder_m0);
        
        // 当角度变化超过 0.05 rad (~3度) 时打印
        if (fabs(current_angle - last_logged_angle) > 0.05f) {
            log_d("Encoder Angle: %.2f rad", current_angle);
            last_logged_angle = current_angle;
        }
    }
}

void User_Task_2(void)
{
    static pwm_led_t *pwm_m0_in_1 = NULL;
    static uint32_t pwm_update_tick = 0;    // 用于PWM更新的时间跟踪
    static uint32_t log_print_tick = 0;     // 用于日志打印的时间跟踪
    static int8_t direction = 1;
    static uint8_t brightness = 0;
    
    // 初始化 PWM LED（只执行一次）
    if (pwm_m0_in_1 == NULL) {
        pwm_driver_t *driver = pwm_driver_get(M0_IN_1);
        
        if (driver == NULL) {
            log_e("M0_IN_1 PWM driver not found");
            vTaskDelay(500);
            return;
        }
        
        // 创建 PWM LED，频率 1kHz，高电平有效
        pwm_m0_in_1 = pwm_led_create(1000, driver, 1);
        
        if (pwm_m0_in_1 == NULL) {
            log_e("Failed to create PWM LED for M0_IN_1");
            vTaskDelay(500);
            return;
        }
        
        log_i("M0_IN_1 PWM initialized at 1kHz");
    }

    // 每 500ms 打印一次亮度
    if (sys_get_systick_ms() - log_print_tick >= 500) {
        log_print_tick = sys_get_systick_ms();
        log_i("M0_IN_1 PWM brightness: %d", brightness);
    }

    // 每 20ms 更新一次亮度，实现呼吸灯效果
    if (sys_get_systick_ms() - pwm_update_tick >= 20) {
        pwm_update_tick = sys_get_systick_ms();
        
        pwm_led_set_brightness(pwm_m0_in_1, brightness);
        
        brightness += direction;
        if (brightness >= 100 || brightness <= 0) {
            direction = -direction;
        }
    } 
    
    // 短暂延时，避免占用过多CPU
    vTaskDelay(1);
}

int fputc(int ch, FILE *f)
{
    uint8_t data = (uint8_t)ch;
    if (g_debug_queue) {
        // 使用异步队列发送，解决与 DMA 的冲突并防止阻塞任务
        uart_queue_send(g_debug_queue, &data, 1);
    } else {
        // 队列未就绪时降级使用阻塞发送
        HAL_UART_Transmit(&huart1, &data, 1, 10);
    }
    return ch;
}
