/*
 * BSP_init.c
 *
 *  Created on: Feb 7, 2026
 *      Author: 12114
 */

#if 1

#include "dev_map_config.h"
#include "dev_map.h"

#include "Component_inc.h"
#include "factory_inc.h"
#include "interface_inc.h"

#include "elog.h"

#include "ThreePhase_motor\ThreePhase_motor.h"
#include "motor_control\motor_control.h"

/* 全局设备句柄 */
uart_queue_t *g_debug_queue = NULL;
motor_control_t *Motor_1_control = NULL;
motor_control_t *Motor_2_control = NULL;
absolute_encoder_driver_t *g_encoder_m0 = NULL;
absolute_encoder_driver_t *g_encoder_m1 = NULL;

/* 调试串口队列用的缓冲区 */
static uint8_t debug_tx_buf[2048];
static uint8_t debug_rx_buf[64];

void bsp_init(void)
{
  /* 系统初始化 */
  sys_init();
#ifndef platform_sys_mem_create
#warning "mem: 未定义有效的目标平台，请检查TARGET_PLATFORM配置"
#else
  SysMem *mem_temp = NULL;
  mem_temp = platform_sys_mem_create();
  sys_mem_init(mem_temp); // 内存池功能初始化

  sys_mem_init_internal();
#endif
  /* 1. 创建硬件抽象层串口对象 */
  usart_driver_t *g_debug_uart = usart_driver_get(USART_ID_DEBUG);

  /* 2. 创建并初始化日志串口队列 */
  if (g_debug_uart)
  {
    g_debug_queue =
        uart_queue_create(g_debug_uart, debug_tx_buf, sizeof(debug_tx_buf),
                          debug_rx_buf, sizeof(debug_rx_buf));
    if (g_debug_queue)
    {
      uart_queue_set_wait_config(g_debug_queue, 2, 50); // 2ms等待, 最多50次
      uart_queue_set_auto_wait(g_debug_queue, true);
      uart_queue_start_receive(g_debug_queue); // 开启异步接收
    }
  }
  /* 3. elog初始化 */
  if (elog_init_and_config() == ELOG_NO_ERR)
  {
    log_i("log init success.");
    log_a("log lvel: %d", ELOG_LVL_TOTAL_NUM);
    log_i("sys CoreClock: %d MHz", sys_get_CoreClock());
  }
  else
  {
    elog_deinit();
  }

  pwm_driver_t *M_IN_1 = pwm_driver_get(M0_IN_1);
  pwm_driver_t *M_IN_2 = pwm_driver_get(M0_IN_2);
  pwm_driver_t *M_IN_3 = pwm_driver_get(M0_IN_3);

  pwm_driver_t *M1_IN_1_drv = pwm_driver_get(M1_IN_1);
  pwm_driver_t *M1_IN_2_drv = pwm_driver_get(M1_IN_2);
  pwm_driver_t *M1_IN_3_drv = pwm_driver_get(M1_IN_3);

  PWM_SET_FREQ(M_IN_1, 30000);
  PWM_SET_FREQ(M_IN_2, 30000);
  PWM_SET_FREQ(M_IN_3, 30000);

  PWM_SET_FREQ(M1_IN_1_drv, 30000);
  PWM_SET_FREQ(M1_IN_2_drv, 30000);
  PWM_SET_FREQ(M1_IN_3_drv, 30000);

  threephase_motor_t *Motor_1 = threephase_motor_create(M_IN_1, M_IN_2, M_IN_3, 12.0f);
  motor_set_voltage(Motor_1, 12.0f, 12.0f);
  Motor_1_control = motor_control_create(Motor_1, 7);

  threephase_motor_t *Motor_2 = threephase_motor_create(M1_IN_1_drv, M1_IN_2_drv, M1_IN_3_drv, 12.0f);
  motor_set_voltage(Motor_2, 12.0f, 12.0f);
  Motor_2_control = motor_control_create(Motor_2, 7);

  /* 4. 创建编码器驱动 */
  g_encoder_m0 = absolute_encoder_driver_get(ENCODER_ID_M0);
  if (g_encoder_m0) {
      log_i("Encoder M0 initialized.");
  } else {
      log_w("Encoder M0 initialization failed!");
  }

  g_encoder_m1 = absolute_encoder_driver_get(ENCODER_ID_M1);
  if (g_encoder_m1) {
      log_i("Encoder M1 initialized.");
  } else {
      log_w("Encoder M1 initialization failed!");
  }
}

#endif
