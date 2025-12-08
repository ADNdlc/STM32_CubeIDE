/*
 * uart_queue_test.c
 *
 *  Created on: Dec 3, 2025
 *      Author: 12114
 */
#include "all_tests_config.h"
#if _uart_queue_test_

#include "device_mapping.h"
#include "main.h"
#include "uart_queue/uart_queue.h"
#include "uart_queue_test.h"
#include "usart_factory.h"
#include "usart_hal/usart_hal.h"
#include <stdio.h>
#include <string.h>

// 延时辅助函数
static void delay_ms(uint32_t ms) { HAL_Delay(ms); }

// 定义缓冲区大小
#define TX_BUFFER_SIZE 2048
#define RX_BUFFER_SIZE 2048

// 静态缓冲区
static uint8_t tx_buffer[TX_BUFFER_SIZE];
static uint8_t rx_buffer[RX_BUFFER_SIZE];

// 1. UART 队列基本发送测试
static void test_uart_queue_basic_send(void)
{
  // 获取 USART 驱动实例
  usart_driver_t *driver = usart_driver_get(USART_LOGGER);

  if (driver)
  {
    // 创建 USART HAL 对象
    usart_hal_t *uart_hal = usart_hal_create(driver);

    if (uart_hal)
    {
      // 初始化 UART 队列
      uart_queue_t queue;
      uart_queue_init(&queue, uart_hal, tx_buffer, TX_BUFFER_SIZE, rx_buffer,
                      RX_BUFFER_SIZE);

      // 测试发送单条消息
      const char *msg1 = "UART Queue Test: Message 1\r\n";
      bool result =
          uart_queue_send(&queue, (const uint8_t *)msg1, strlen(msg1));

      if (result)
      {
        // 等待发送完成
        delay_ms(100);

        // 发送第二条消息
        const char *msg2 = "UART Queue Test: Message 2\r\n";
        uart_queue_send(&queue, (const uint8_t *)msg2, strlen(msg2));
        delay_ms(100);
      }

      // 销毁对象
      usart_hal_destroy(uart_hal);
    }
  }
}

// 2. UART 队列批量发送测试
static void test_uart_queue_batch_send(void)
{
  // 获取 USART 驱动实例
  usart_driver_t *driver = usart_driver_get(USART_LOGGER);

  if (driver)
  {
    // 创建 USART HAL 对象
    usart_hal_t *uart_hal = usart_hal_create(driver);

    if (uart_hal)
    {
      // 初始化 UART 队列
      uart_queue_t queue;
      uart_queue_init(&queue, uart_hal, tx_buffer, TX_BUFFER_SIZE, rx_buffer,
                      RX_BUFFER_SIZE);

      // 批量发送多条消息
      for (int i = 0; i < 4; i++)
      {
        char msg[50];
        snprintf(msg, sizeof(msg), "Batch message #%d\r\n", i + 1);
        uart_queue_send(&queue, (const uint8_t *)msg, strlen(msg));
      }

      delay_ms(1);

      // 检查队列状态
      size_t remaining = uart_queue_tx_count(&queue); // 不延时剩余72,延时1ms剩54
      char status_msg[50];
      snprintf(status_msg, sizeof(status_msg),
               "TX Queue remaining: %u bytes\r\n", (unsigned int)remaining);
      uart_queue_send(&queue, (const uint8_t *)status_msg, strlen(status_msg));

      delay_ms(1000);

      // 销毁对象
      usart_hal_destroy(uart_hal);
    }
  }
}

// 3. UART 队列缓冲区溢出测试
static void test_uart_queue_overflow(void)
{
  // 获取 USART 驱动实例
  usart_driver_t *driver = usart_driver_get(USART_LOGGER);

  if (driver)
  {
    // 创建 USART HAL 对象
    usart_hal_t *uart_hal = usart_hal_create(driver);

    if (uart_hal)
    {
      // 初始化 UART 队列
      uart_queue_t queue;
      uart_queue_init(&queue, uart_hal, tx_buffer, TX_BUFFER_SIZE, rx_buffer,
                      RX_BUFFER_SIZE);

      const char *test_msg = "Overflow test: ";
      uart_queue_send(&queue, (const uint8_t *)test_msg, strlen(test_msg));
      delay_ms(50);

      // 尝试快速填满缓冲区
      int success_count = 0;
      int fail_count = 0;

      for (int i = 1; i < 21; i++)
      {
        char msg[30];
        snprintf(msg, sizeof(msg), "Data block %d\r\n", i);

        if (uart_queue_send(&queue, (const uint8_t *)msg, strlen(msg)))
        {
          success_count++;
        }
        else
        {
          fail_count++;
        }
      }

      // 报告结果
      delay_ms(200);
      char result_msg[80];
      snprintf(result_msg, sizeof(result_msg),
               "Overflow test: %d success, %d failed\r\n", success_count,
               fail_count);
      uart_queue_send(&queue, (const uint8_t *)result_msg, strlen(result_msg));

      // 等待队列清空
      delay_ms(1000);

      // 销毁对象
      usart_hal_destroy(uart_hal);
    }
  }
}

// 4. UART 队列接收测试
static void test_uart_queue_receive(void)
{
  // 获取 USART 驱动实例
  usart_driver_t *driver = usart_driver_get(USART_LOGGER);

  if (driver)
  {
    // 创建 USART HAL 对象
    usart_hal_t *uart_hal = usart_hal_create(driver);

    if (uart_hal)
    {
      // 初始化 UART 队列
      uart_queue_t queue;
      uart_queue_init(&queue, uart_hal, tx_buffer, TX_BUFFER_SIZE, rx_buffer,
                      RX_BUFFER_SIZE);

      // 发送提示信息
      const char *prompt = "RX test: Please send data within 5 seconds...\r\n";
      uart_queue_send(&queue, (const uint8_t *)prompt, strlen(prompt));
      delay_ms(100);

      // 启动接收
      uart_queue_start_receive(&queue);

      // 等待接收数据（5秒）
      uint32_t timeout = 5000;
      uint32_t elapsed = 0;

      while (elapsed < timeout)
      {
        size_t rx_count = uart_queue_rx_count(&queue);

        if (rx_count > 0)
        {
          // 有数据可读
          uint8_t read_buf[2048];
          size_t read_len =
              uart_queue_getdata(&queue, read_buf, sizeof(read_buf) - 1);

          if (read_len > 0)
          {
            // 回显接收到的数据
            read_buf[read_len] = '\0'; // 添加字符串结束符

            const char *echo_prefix = "Received: ";
            uart_queue_send(&queue, (const uint8_t *)echo_prefix,
                            strlen(echo_prefix));
            uart_queue_send(&queue, read_buf, read_len);

            const char *newline = "\r\n";
            uart_queue_send(&queue, (const uint8_t *)newline, strlen(newline));

            delay_ms(100);
          }
        }

        delay_ms(100);
        elapsed += 100;
      }

      const char *timeout_msg = "RX test: Timeout\r\n";
      uart_queue_send(&queue, (const uint8_t *)timeout_msg,
                      strlen(timeout_msg));
      delay_ms(100);

      // 销毁对象
      usart_hal_destroy(uart_hal);
    }
  }
}

// 5. UART 队列状态查询测试
static void test_uart_queue_status(void)
{
  // 获取 USART 驱动实例
  usart_driver_t *driver = usart_driver_get(USART_LOGGER);

  if (driver)
  {
    // 创建 USART HAL 对象
    usart_hal_t *uart_hal = usart_hal_create(driver);

    if (uart_hal)
    {
      // 初始化 UART 队列
      uart_queue_t queue;
      uart_queue_init(&queue, uart_hal, tx_buffer, TX_BUFFER_SIZE, rx_buffer,
                      RX_BUFFER_SIZE);

      // 测试初始状态
      size_t tx_count = uart_queue_tx_count(&queue);
      size_t rx_count = uart_queue_rx_count(&queue);

      char status_msg[80];
      snprintf(status_msg, sizeof(status_msg),
               "Initial state - TX: %u, RX: %u\r\n", (unsigned int)tx_count,
               (unsigned int)rx_count);
      uart_queue_send(&queue, (const uint8_t *)status_msg, strlen(status_msg));
      delay_ms(100);

      // 添加一些数据后检查状态
      const char *test_data = "Status test data\r\n";
      uart_queue_send(&queue, (const uint8_t *)test_data, strlen(test_data));

      tx_count = uart_queue_tx_count(&queue);
      snprintf(status_msg, sizeof(status_msg), "Before send - TX count: %u\r\n", //tx_count==18
               (unsigned int)tx_count);
      uart_queue_send(&queue, (const uint8_t *)status_msg, strlen(status_msg));

      delay_ms(100);
      tx_count = uart_queue_tx_count(&queue);
      snprintf(status_msg, sizeof(status_msg), "After send - TX count: %u\r\n", //tx_count==0
               (unsigned int)tx_count);
      uart_queue_send(&queue, (const uint8_t *)status_msg, strlen(status_msg));

      delay_ms(200);

      // 销毁对象
      usart_hal_destroy(uart_hal);
    }
  }
}

// 主测试入口
void uart_queue_test_run(void)
{
  // 测试开始提示
  usart_driver_t *driver = usart_driver_get(USART_LOGGER);
  if (driver)
  {
    usart_hal_t *uart_hal = usart_hal_create(driver);
    if (uart_hal)
    {
      const char *start_msg =
          "\r\n========== UART Queue Test Start ==========\r\n";
      usart_hal_send(uart_hal, (const uint8_t *)start_msg, strlen(start_msg),
                     1000);
      usart_hal_destroy(uart_hal);
    }
  }

  delay_ms(100);

  // 运行各项测试
//  test_uart_queue_basic_send();
//  delay_ms(500);
//
//  test_uart_queue_batch_send();
//  delay_ms(500);
//
//  test_uart_queue_overflow();
//  delay_ms(500);
//
//  test_uart_queue_status();
//  delay_ms(500);

  test_uart_queue_receive();
  delay_ms(500);

  // 测试结束提示
  driver = usart_driver_get(USART_LOGGER);
  if (driver)
  {
    usart_hal_t *uart_hal = usart_hal_create(driver);
    if (uart_hal)
    {
      const char *end_msg = "========== UART Queue Test End ==========\r\n\r\n";
      usart_hal_send(uart_hal, (const uint8_t *)end_msg, strlen(end_msg), 1000);
      usart_hal_destroy(uart_hal);
    }
  }
}

#endif
