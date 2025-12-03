/*
 * all_tests.c
 *
 *  Created on: Dec 3, 2025
 *      Author: 12114
 */

#include "all_tests.h"
#include "led_test_app.h"
#include "main.h"
#include "uart_hal_test.h"
#include "uart_queue_test.h"


// 延时辅助函数
static void delay_ms(uint32_t ms) { HAL_Delay(ms); }

/**
 * @brief 运行所有测试用例
 *
 * 此函数按顺序执行所有可用的测试用例：
 * 1. LED 测试 (GPIO LED, PWM LED, RGB LED)
 * 2. UART HAL 测试 (同步/异步发送接收)
 * 3. UART Queue 测试 (队列发送、接收、溢出处理)
 */
void run_all_tests(void) {
  // 1. LED 测试
  led_test_run();
  delay_ms(1000);

  // 2. UART HAL 测试
  uart_hal_test_run();
  delay_ms(1000);

  // 3. UART Queue 测试
  uart_queue_test_run();
  delay_ms(1000);
}
