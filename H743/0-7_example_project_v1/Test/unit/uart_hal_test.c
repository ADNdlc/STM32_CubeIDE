/*
 * uart_hal_test.c
 *
 *  Created on: Dec 3, 2025
 *      Author: 12114
 */
#include "all_tests_config.h"
#if uart_hal_test 

#include "uart_hal_test.h"
#include "device_mapping.h"
#include "main.h"
#include "usart_factory.h"
#include "usart_hal/usart_hal.h"
#include <string.h>


// 延时辅助函数
static void delay_ms(uint32_t ms) { HAL_Delay(ms); }

// 1. UART HAL 同步发送测试
static void test_uart_hal_sync_transmit(void) {
  // 获取 USART 驱动实例
  usart_driver_t *driver = usart_driver_get(USART_LOGGER);

  if (driver) {
    // 创建 USART HAL 对象
    usart_hal_t *uart_hal = usart_hal_create(driver);

    if (uart_hal) {
      // 测试同步发送
      const char *test_msg = "UART HAL Sync Transmit Test\r\n";
      int result = usart_hal_send(uart_hal, (const uint8_t *)test_msg,
                                  strlen(test_msg), 1000);

      if (result == 0) {
        // 发送成功提示
        const char *success_msg = "Sync transmit: OK\r\n";
        usart_hal_send(uart_hal, (const uint8_t *)success_msg,
                       strlen(success_msg), 1000);
      } else {
        // 发送失败提示
        const char *fail_msg = "Sync transmit: FAILED\r\n";
        usart_hal_send(uart_hal, (const uint8_t *)fail_msg, strlen(fail_msg),
                       1000);
      }

      delay_ms(100);

      // 销毁对象
      usart_hal_destroy(uart_hal);
    }
  }
}

// 2. UART HAL 同步接收测试
static void test_uart_hal_sync_receive(void) {
  // 获取 USART 驱动实例
  usart_driver_t *driver = usart_driver_get(USART_LOGGER);

  if (driver) {
    // 创建 USART HAL 对象
    usart_hal_t *uart_hal = usart_hal_create(driver);

    if (uart_hal) {
      const char *prompt_msg = "Please send 10 bytes within 10 seconds...\r\n";
      usart_hal_send(uart_hal, (const uint8_t *)prompt_msg, strlen(prompt_msg),
                     1000);

      // 测试同步接收
      uint8_t rx_buffer[10];
      int result = usart_hal_recv(uart_hal, rx_buffer, sizeof(rx_buffer), 10000); // 等待10秒

      if (result == 0) {
        // 接收成功，回显数据
        const char *echo_msg = "Received: ";
        usart_hal_send(uart_hal, (const uint8_t *)echo_msg, strlen(echo_msg),
                       1000);
        usart_hal_send(uart_hal, rx_buffer, sizeof(rx_buffer), 1000);
        const char *newline = "\r\n";
        usart_hal_send(uart_hal, (const uint8_t *)newline, strlen(newline),
                       1000);
      } else {
        // 接收超时或失败
        const char *timeout_msg = "Sync receive: TIMEOUT or FAILED\r\n";
        usart_hal_send(uart_hal, (const uint8_t *)timeout_msg,
                       strlen(timeout_msg), 1000);
      }

      delay_ms(100);

      // 销毁对象
      usart_hal_destroy(uart_hal);
    }
  }
}

// 回调函数用于异步测试
static volatile uint8_t async_tx_complete = 0;
static volatile uint8_t async_rx_complete = 0;

static void uart_test_callback(void *context, usart_event_t event, void *args) {
  (void)context;
  (void)args;

  switch (event) {
  case USART_EVENT_TX_COMPLETE:
    async_tx_complete = 1;
    break;

  case USART_EVENT_RX_EVENT:
    async_rx_complete = 1;
    break;

  case USART_EVENT_ERROR:
    // 错误处理
    break;

  default:
    break;
  }
}

// 3. UART HAL 异步发送测试
static void test_uart_hal_async_transmit(void) {
  // 获取 USART 驱动实例
  usart_driver_t *driver = usart_driver_get(USART_LOGGER);

  if (driver) {
    // 创建 USART HAL 对象
    usart_hal_t *uart_hal = usart_hal_create(driver);

    if (uart_hal) {
      // 设置回调函数
      usart_hal_set_callback(uart_hal, uart_test_callback, NULL);

      // 测试异步发送
      async_tx_complete = 0;
      const char *test_msg = "UART HAL Async Transmit Test\r\n";
      int result = usart_hal_transmit_asyn(uart_hal, (const uint8_t *)test_msg,
                                           strlen(test_msg));

      if (result == 0) {
        // 等待发送完成（最多1秒）
        uint32_t timeout = 1000;
        while (!async_tx_complete && timeout > 0) {
          delay_ms(10);
          timeout -= 10;
        }

        if (async_tx_complete) {
          const char *success_msg = "Async transmit: OK\r\n";
          usart_hal_send(uart_hal, (const uint8_t *)success_msg,
                         strlen(success_msg), 1000);
        } else {
          const char *timeout_msg = "Async transmit: TIMEOUT\r\n";
          usart_hal_send(uart_hal, (const uint8_t *)timeout_msg,
                         strlen(timeout_msg), 1000);
        }
      } else {
        const char *fail_msg = "Async transmit: FAILED to start\r\n";
        usart_hal_send(uart_hal, (const uint8_t *)fail_msg, strlen(fail_msg),
                       1000);
      }

      delay_ms(100);

      // 销毁对象
      usart_hal_destroy(uart_hal);
    }
  }
}

// 4. UART HAL 异步接收测试
static void test_uart_hal_async_receive(void) {
  // 获取 USART 驱动实例
  usart_driver_t *driver = usart_driver_get(USART_LOGGER);

  if (driver) {
    // 创建 USART HAL 对象
    usart_hal_t *uart_hal = usart_hal_create(driver);

    if (uart_hal) {
      // 设置回调函数
      usart_hal_set_callback(uart_hal, uart_test_callback, NULL);

      const char *prompt_msg =
          "Async receive: Please send 10 bytes within 5 seconds...\r\n";
      usart_hal_send(uart_hal, (const uint8_t *)prompt_msg, strlen(prompt_msg),
                     1000);

      // 测试异步接收
      async_rx_complete = 0;
      uint8_t rx_buffer[10];
      int result =
          usart_hal_receive_asyn(uart_hal, rx_buffer, sizeof(rx_buffer));

      if (result == 0) {
        // 等待接收完成（最多5秒）
        uint32_t timeout = 5000;
        while (!async_rx_complete && timeout > 0) {
          delay_ms(10);
          timeout -= 10;
        }

        if (async_rx_complete) {
          // 接收成功，回显数据
          const char *echo_msg = "Async received: ";
          usart_hal_send(uart_hal, (const uint8_t *)echo_msg, strlen(echo_msg),
                         1000);
          usart_hal_send(uart_hal, rx_buffer, sizeof(rx_buffer), 1000);
          const char *newline = "\r\n";
          usart_hal_send(uart_hal, (const uint8_t *)newline, strlen(newline),
                         1000);
        } else {
          const char *timeout_msg = "Async receive: TIMEOUT\r\n";
          usart_hal_send(uart_hal, (const uint8_t *)timeout_msg,
                         strlen(timeout_msg), 1000);
        }
      } else {
        const char *fail_msg = "Async receive: FAILED to start\r\n";
        usart_hal_send(uart_hal, (const uint8_t *)fail_msg, strlen(fail_msg),
                       1000);
      }

      delay_ms(100);

      // 销毁对象
      usart_hal_destroy(uart_hal);
    }
  }
}

// 主测试入口
void uart_hal_test_run(void) {
  // 测试开始提示
  usart_driver_t *driver = usart_driver_get(USART_LOGGER);
  if (driver) {
    usart_hal_t *uart_hal = usart_hal_create(driver);
    if (uart_hal) {
      const char *start_msg =
          "\r\n========== UART HAL Test Start ==========\r\n";
      usart_hal_send(uart_hal, (const uint8_t *)start_msg, strlen(start_msg),
                     1000);
      usart_hal_destroy(uart_hal);
    }
  }

  // 运行各项测试
  test_uart_hal_sync_transmit();
  delay_ms(500);

  test_uart_hal_sync_receive();
  delay_ms(500);

  test_uart_hal_async_transmit();
  delay_ms(500);

  test_uart_hal_async_receive();
  delay_ms(500);

  // 测试结束提示
  driver = usart_driver_get(USART_LOGGER);
  if (driver) {
    usart_hal_t *uart_hal = usart_hal_create(driver);
    if (uart_hal) {
      const char *end_msg = "========== UART HAL Test End ==========\r\n\r\n";
      usart_hal_send(uart_hal, (const uint8_t *)end_msg, strlen(end_msg), 1000);
      usart_hal_destroy(uart_hal);
    }
  }
}

#endif
