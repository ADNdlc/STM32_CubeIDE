#include "test_config.h"
#if ENABLE_TEST_USART_QUEUE
#include "test_framework.h"
#include "uart_queue/uart_queue.h"
#include "usart_factory.h"
#include <stdio.h>
#include <string.h>


static void test_uart_queue_basic_send(void);
static void test_uart_queue_batch_send(void);
static void test_uart_queue_overflow(void);
static void test_uart_queue_receive(void);
static void test_uart_queue_status(void);

#define TEST_USART USART_ID_ESP8266
// 定义缓冲区大小
#define TX_BUFFER_SIZE 256
#define RX_BUFFER_SIZE 256

// 静态缓冲区
static uint8_t *tx_buffer;
static uint8_t *rx_buffer;
static usart_driver_t *test_usart;
static uart_queue_t *queue;
static void test_uart_queue_setup(void) {
  tx_buffer = sys_malloc(SYS_MEM_INTERNAL, TX_BUFFER_SIZE);
  tx_buffer = sys_malloc(SYS_MEM_INTERNAL, RX_BUFFER_SIZE);
  test_usart = usart_driver_get(TEST_USART);
  // 初始化 UART 队列
  queue = uart_queue_create(test_usart, tx_buffer, TX_BUFFER_SIZE, rx_buffer,
                            RX_BUFFER_SIZE);
}

static void test_uart_queue_loop(void) {
  // 测试开始提示
  if (test_usart) {
    const char *start_msg =
        "\r\n========== UART Queue Test Start ==========\r\n";
    USART_TRANSMIT(test_usart, (const uint8_t *)start_msg, strlen(start_msg),
                   1000);
  }

  sys_delay_ms(100);

  // 运行各项测试
  test_uart_queue_basic_send();
  sys_delay_ms(200);

  test_uart_queue_batch_send();
  sys_delay_ms(200);

  test_uart_queue_overflow();
  sys_delay_ms(200);

  test_uart_queue_status();
  sys_delay_ms(200);

  test_uart_queue_receive();
  sys_delay_ms(200);

  const char *end_msg = "========== UART Queue Test End ==========\r\n\r\n";
  USART_TRANSMIT(test_usart, (const uint8_t *)end_msg, strlen(end_msg), 1000);
  sys_delay_ms(10000);
}

static void test_uart_queue_teardown(void) {
  sys_free(SYS_MEM_INTERNAL, tx_buffer);
  sys_free(SYS_MEM_INTERNAL, rx_buffer);
  uart_queue_destroy(queue);
  const char *teardown_msg = "Test completed.\r\n";
  USART_TRANSMIT(test_usart, (const uint8_t *)teardown_msg,
                 strlen(teardown_msg), 1000);
}

REGISTER_TEST(USART_QUEUE,
              "Verify the complete functionality of the serial port queue",
              test_uart_queue_setup, test_uart_queue_loop,
              test_uart_queue_teardown);

// 1. UART 队列基本发送测试
static void test_uart_queue_basic_send(void) {
  if (test_usart) {
    // 测试发送单条消息
    const char *msg1 = "UART Queue Test: Message 1\r\n";
    bool result = uart_queue_send(queue, (const uint8_t *)msg1, strlen(msg1));

    if (result) {
      // 等待发送完成
      sys_delay_ms(100);

      // 发送第二条消息
      const char *msg2 = "UART Queue Test: Message 2\r\n";
      uart_queue_send(queue, (const uint8_t *)msg2, strlen(msg2));
      sys_delay_ms(100);
    }
  }
}

// 2. UART 队列批量发送测试
static void test_uart_queue_batch_send(void) {
  if (test_usart) {
    // 批量发送多条消息
    for (int i = 0; i < 4; i++) {
      char msg[50];
      snprintf(msg, sizeof(msg), "Batch message #%d\r\n", i + 1);
      uart_queue_send(queue, (const uint8_t *)msg, strlen(msg));
    }

    sys_delay_ms(1);

    // 检查队列状态
    size_t remaining = uart_queue_tx_count(queue); // 不延时剩余72,延时1ms剩54
    char status_msg[50];
    snprintf(status_msg, sizeof(status_msg), "TX Queue remaining: %u bytes\r\n",
             (unsigned int)remaining);
    uart_queue_send(queue, (const uint8_t *)status_msg, strlen(status_msg));

    sys_delay_ms(1000);
  }
}

// 3. UART 队列缓冲区溢出测试
static void test_uart_queue_overflow(void) {
  if (test_usart) {
    /* -- 自动等待关 -- */
    const char *start_msg = "Overflow test starting...\r\n";
    uart_queue_send(queue, (const uint8_t *)start_msg, strlen(start_msg));
    sys_delay_ms(100);

    uart_queue_set_auto_wait(queue, false);

    int success_count = 0;
    int fail_count = 0;
    for (int i = 1; i < 31; i++) {
      char msg[30];
      snprintf(msg, sizeof(msg), "Data block %d\r\n", i);
      if (uart_queue_send(queue, (const uint8_t *)msg, strlen(msg))) {
        success_count++;
      } else {
        fail_count++;
      }
    }
    // 报告结果
    sys_delay_ms(2000); // 增加等待时间，确保所有数据都被发送完毕
    char result_msg[50];
    snprintf(result_msg, sizeof(result_msg),
             "Overflow test: %d success, %d failed\r\n", success_count,
             fail_count);
    uart_queue_send(queue, (const uint8_t *)result_msg, strlen(result_msg));

    // Allow queue to drain
    sys_delay_ms(2000);

    /* -- 自动等待开 -- */
    start_msg = "Overflow test starting...(Auto wait)\r\n";
    uart_queue_send(queue, (const uint8_t *)start_msg, strlen(start_msg));
    sys_delay_ms(100);

    uart_queue_set_auto_wait(queue, true);

    success_count = 0;
    fail_count = 0;
    for (int i = 1; i < 31; i++) {
      char msg[30];
      snprintf(msg, sizeof(msg), "Data block %d\r\n", i);
      if (uart_queue_send(queue, (const uint8_t *)msg, strlen(msg))) {
        success_count++;
      } else {
        fail_count++;
      }
    }
    // 报告结果
    sys_delay_ms(200);
    snprintf(result_msg, sizeof(result_msg),
             "Overflow test: %d success, %d failed\r\n", success_count,
             fail_count);
    uart_queue_send(queue, (const uint8_t *)result_msg, strlen(result_msg));

    // 等待队列清空
    sys_delay_ms(1000);
  }
}

// 4. UART 队列接收测试
static void test_uart_queue_receive(void) {
  if (test_usart) {
    // 发送提示信息
    const char *prompt = "RX test: Please send data within 5 seconds...\r\n";
    uart_queue_send(queue, (const uint8_t *)prompt, strlen(prompt));
    sys_delay_ms(100);

    // 启动接收
    uart_queue_start_receive(queue);

    // 等待接收数据（5秒）
    uint32_t timeout = 5000;
    uint32_t elapsed = 0;

    while (elapsed < timeout) {
      size_t rx_count = uart_queue_rx_count(queue);

      if (rx_count > 0) {
        // 有数据可读
        uint8_t read_buf[2048];
        size_t read_len =
            uart_queue_getdata(queue, read_buf, sizeof(read_buf) - 1);

        if (read_len > 0) {
          // 回显接收到的数据
          read_buf[read_len] = '\0'; // 添加字符串结束符

          const char *echo_prefix = "Received: ";
          uart_queue_send(queue, (const uint8_t *)echo_prefix,
                          strlen(echo_prefix));
          uart_queue_send(queue, read_buf, read_len);

          const char *newline = "\r\n";
          uart_queue_send(queue, (const uint8_t *)newline, strlen(newline));

          sys_delay_ms(100);
        }
      }

      sys_delay_ms(100);
      elapsed += 100;
    }

    const char *timeout_msg = "RX test: Timeout\r\n";
    uart_queue_send(queue, (const uint8_t *)timeout_msg, strlen(timeout_msg));
    sys_delay_ms(100);
  }
}

// 5. UART 队列状态查询测试
static void test_uart_queue_status(void) {
  if (test_usart) {
    // 测试初始状态
    size_t tx_count = uart_queue_tx_count(queue);
    size_t rx_count = uart_queue_rx_count(queue);

    char status_msg[80];
    snprintf(status_msg, sizeof(status_msg),
             "Initial state - TX: %u, RX: %u\r\n", (unsigned int)tx_count,
             (unsigned int)rx_count);
    uart_queue_send(queue, (const uint8_t *)status_msg, strlen(status_msg));
    sys_delay_ms(100);

    // 添加一些数据后检查状态
    const char *test_data = "Status test data\r\n";
    uart_queue_send(queue, (const uint8_t *)test_data, strlen(test_data));

    tx_count = uart_queue_tx_count(queue);
    snprintf(status_msg, sizeof(status_msg),
             "Before send - TX count: %u\r\n", // tx_count==18
             (unsigned int)tx_count);
    uart_queue_send(queue, (const uint8_t *)status_msg, strlen(status_msg));

    sys_delay_ms(100);
    tx_count = uart_queue_tx_count(queue);
    snprintf(status_msg, sizeof(status_msg),
             "After send - TX count: %u\r\n", // tx_count==0
             (unsigned int)tx_count);
    uart_queue_send(queue, (const uint8_t *)status_msg, strlen(status_msg));

    sys_delay_ms(200);
  }
}

#endif
