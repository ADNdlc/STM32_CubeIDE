/*
 * uart_queue.c
 *
 *  Created on: Dec 3, 2025
 *      Author: 12114
 */

#include "uart_queue.h"
#include "stdio.h"
#include <stdlib.h>
#include "string.h"
#include "sys.h"
#include "MemPool.h"
#ifdef USE_MEMPOOL
#define UARTQUEUE_MEMSOURCE SYS_MEM_INTERNAL // 从哪里分配
#endif

#define Rx_Manual_restart 0

// 定义消息宏
#define WAIT_MESSAGE_PREFIX "\r\n[WAIT:"
#define WAIT_MESSAGE_SUFFIX "ms]\r\n"
#define DROP_MESSAGE "[DROP]\r\n"

// 最大可能的消息长度
// 最长的消息是 "\r\n[WAIT:4294967295ms]\r\n[DROP]\r\n"
#define WARNING_MESSAGELEN 25

/**
 * @brief UART中断回调函数
 *
 * @param context UART队列实例
 * @param event 事件类型
 * @param args 事件参数
 */
static void uart_queue_callback(void *context, usart_event_t event, void *args);

static void uart_queue_handle_tx_complete(uart_queue_t *queue);
static void uart_queue_handle_rx_complete(uart_queue_t *queue);
static void uart_queue_handle_rx_event(uart_queue_t *queue, void *args);
static void uart_queue_try_start_tx(uart_queue_t *queue);

/**
 * @brief 初始化UART队列
 *
 * @param queue 队列实例
 * @param driver USART硬件驱动
 * @param tx_buffer 发送缓冲区
 * @param tx_size 发送缓冲区大小
 * @param rx_buffer 接收缓冲区
 * @param rx_size 接收缓冲区大小
 */
void uart_queue_init(uart_queue_t *queue, const usart_driver_t *driver,
                     uint8_t *tx_buffer, size_t tx_size, uint8_t *rx_buffer,
                     size_t rx_size) {
  rb_init(&queue->tx_rb, tx_buffer, tx_size);
  rb_init(&queue->rx_rb, rx_buffer, rx_size);
  queue->uart_driver = driver;
  queue->tx_busy = false;
  queue->rx_enabled = false;
  queue->tx_current_len = 0;
  queue->last_rx_pos = 0;
  queue->auto_wait = false; // Default to false
  queue->wait_delay_ms = UART_QUEUE_AUTO_WAIT_DELAY_MS;
  queue->wait_max_count = UART_QUEUE_AUTO_WAIT_MAX_COUNT;
  // 设置回调函数
  USART_SET_CALLBACK((usart_driver_t *)queue->uart_driver, uart_queue_callback,
                     queue);
}

uart_queue_t *uart_queue_create(const usart_driver_t *driver,
                                uint8_t *tx_buffer, size_t tx_size,
                                uint8_t *rx_buffer, size_t rx_size) {
#ifdef USE_MEMPOOL
  uart_queue_t *queue =
      (uart_queue_t *)sys_malloc(UARTQUEUE_MEMSOURCE, sizeof(uart_queue_t));
#else
  uart_queue_t *queue =
      (uart_queue_t *)malloc(sizeof(uart_queue_t));
#endif
  if (!queue) {
    return NULL;
  }
  uart_queue_init(queue, driver, tx_buffer, tx_size, rx_buffer, rx_size);
  return queue;
}

void uart_queue_destroy(uart_queue_t *queue) {
  if (queue) {
#ifdef USE_MEMPOOL
    sys_free(UARTQUEUE_MEMSOURCE, queue);
#else
    free(queue);
#endif
  }
}

// 设置是否启用发送缓冲区满自动等待
void uart_queue_set_auto_wait(uart_queue_t *queue, bool enabled) {
  if (queue) {
    queue->auto_wait = enabled;
  }
}

void uart_queue_set_wait_config(uart_queue_t *queue, uint32_t delay_ms,
                                uint16_t max_count) {
  if (queue) {
    queue->wait_delay_ms = delay_ms;
    queue->wait_max_count = max_count;
  }
}

/**
 * @brief UART中断回调函数(依赖在对应平台的uart_driver里传入缓冲区的写入长度)
 *
 * @param context UART队列实例
 * @param event 事件类型
 * @param args 事件参数
 */
static void uart_queue_callback(void *context, usart_event_t event,
                                void *args) {
  uart_queue_t *queue = (uart_queue_t *)context;

  // test
  // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);

  switch (event) {
  case USART_EVENT_RX_COMPLETE:
    uart_queue_handle_rx_complete(queue);
    break;

  case USART_EVENT_TX_COMPLETE:
    uart_queue_handle_tx_complete(queue);
    break;

  case USART_EVENT_RX_EVENT:
    uart_queue_handle_rx_event(queue, args);
    break;

  case USART_EVENT_ERROR:
    // 错误处理
    break;
  }
}

/**
 * @brief 尝试启动发送
 *
 * @param queue 队列实例
 */
static void uart_queue_try_start_tx(uart_queue_t *queue) {
  if (!queue->tx_busy) { // 检查是否忙
    uint8_t *tx_ptr;
    size_t tx_len = rb_peek(&queue->tx_rb, &tx_ptr);
    if (tx_len > 0) {
      queue->tx_busy = true;
      queue->tx_current_len = tx_len;
      if (USART_TRANSMIT_ASYN((usart_driver_t *)queue->uart_driver, tx_ptr,
                              tx_len) != 0) {
        queue->tx_busy = false;
        queue->tx_current_len = 0;
      }
    }
  }
}

/**
 * @brief 发送数据到队列
 *
 * @param queue 队列实例
 * @param data 要发送的数据
 * @param len 数据长度
 * @return bool 是否成功加入队列
 */
bool uart_queue_send(uart_queue_t *queue, const uint8_t *data, size_t len) {
  if (queue->auto_wait) {
    // 检查是否有足够的空间来存放数据
    size_t free_space = rb_free_space(&queue->tx_rb);

    // 如果空间不足，开始尝试等待
    if (free_space < (len + WARNING_MESSAGELEN)) {
      uint32_t start_time = sys_get_systick_ms();
      int count = queue->wait_max_count;

      // 等待 设定时间 直到有 足够空间 或 超过设定超时时间
      while (free_space < (len + WARNING_MESSAGELEN) && count > 0) {
        sys_delay_ms(queue->wait_delay_ms);
        free_space = rb_free_space(&queue->tx_rb);
        count--;
      }

      uint32_t total_wait = sys_get_systick_ms() - start_time;
      char status_msg[WARNING_MESSAGELEN];
      size_t status_len;

      // 如果等待后空间仍然不足，说明彻底溢出
      if (free_space < (len + WARNING_MESSAGELEN)) {
        // 彻底溢出：将 WAIT 和 DROP 合并为一条消息，防止分两次写入导致的截断
        status_len =
            snprintf(status_msg, sizeof(status_msg),
                     WAIT_MESSAGE_PREFIX "%lu" WAIT_MESSAGE_SUFFIX DROP_MESSAGE,
                     total_wait);
        rb_write(&queue->tx_rb, (const uint8_t *)status_msg, status_len);
        uart_queue_try_start_tx(queue);
        return false;
      } else {
        // 等待成功：写入带时间的 WAIT 标记
        status_len =
            snprintf(status_msg, sizeof(status_msg),
                     WAIT_MESSAGE_PREFIX "%lu" WAIT_MESSAGE_SUFFIX, total_wait);
        // 只有空间足以容纳 [状态消息 + payload] 时才写入状态消息，否则优先保
        // payload 以防乱码
        if (rb_free_space(&queue->tx_rb) >= (len + status_len)) {
          rb_write(&queue->tx_rb, (const uint8_t *)status_msg, status_len);
        }
      }
    }
  }
  size_t written = rb_write(&queue->tx_rb, data, len);
  if (written > 0) {
    uart_queue_try_start_tx(queue);
  }
  return (written == len);
}

/**
 * @brief 从接收队列读取数据
 *
 * @param queue 队列实例
 * @param data 存储接收数据的缓冲区
 * @param max_len 缓冲区最大长度
 * @return size_t 实际读取的数据长度
 */
size_t uart_queue_getdata(uart_queue_t *queue, uint8_t *data, size_t max_len) {
  return rb_read(&queue->rx_rb, data, max_len);
}

/**
 * @brief 获取发送队列中可用数据的数量
 *
 * @param queue 队列实例
 * @return size_t 发送队列中的数据数量
 */
size_t uart_queue_tx_count(uart_queue_t *queue) {
  return rb_available(&queue->tx_rb);
}

/**
 * @brief 获取接收队列中可用数据的数量
 *
 * @param queue 队列实例
 * @return size_t 接收队列中的数据数量
 */
size_t uart_queue_rx_count(uart_queue_t *queue) {
  return rb_available(&queue->rx_rb);
}

/**
 * @brief 启动接收
 *
 * @param queue 队列实例
 */
void uart_queue_start_receive(uart_queue_t *queue) {
  if (!queue->rx_enabled) {
    queue->rx_enabled = true;

    // 启动异步接收 - 使用完整缓冲区以支持循环DMA
    // 注意：如果是循环模式，DMA将始终在整个缓冲区上循环，这与RingBuffer的逻辑大小一致
    if (queue->rx_rb.size > 0) {
      USART_RECEIVE_ASYN((usart_driver_t *)queue->uart_driver,
                         queue->rx_rb.buffer, queue->rx_rb.size);
    }
  }
}

/**
 * @brief 处理发送完成事件
 *
 * @param queue 队列实例
 */
static void uart_queue_handle_tx_complete(uart_queue_t *queue) {
  // 更新发送缓冲区的读指针 - 使用上次发送的实际长度
  if (queue->tx_current_len > 0) {
    rb_advance(&queue->tx_rb, queue->tx_current_len);
    queue->tx_current_len = 0;
  }

  uint8_t *tx_ptr;
  // 检查是否还有数据需要发送
  size_t tx_len = rb_peek(&queue->tx_rb, &tx_ptr);
  if (tx_len > 0) {
    queue->tx_current_len = tx_len;
    USART_TRANSMIT_ASYN((usart_driver_t *)queue->uart_driver, tx_ptr, tx_len);
  } else {
    queue->tx_busy = false;
  }
}

/**
 * @brief 处理接收完成事件
 *
 * @param queue 队列实例
 */
static void uart_queue_handle_rx_complete(uart_queue_t *queue) {
  // 重新启动接收
  if (queue->rx_enabled) {
    uint8_t *rx_ptr;
    size_t available_len = rb_peek_write(&queue->rx_rb, &rx_ptr);
    if (available_len > 0) {
      USART_RECEIVE_ASYN((usart_driver_t *)queue->uart_driver, rx_ptr,
                         available_len);
    }
  }
}

/**
 * @brief 处理接收数据事件
 *
 * @param queue 队列实例
 * @param data 接收到的数据
 */
static void uart_queue_handle_rx_event(uart_queue_t *queue, void *args) {
  if (NULL == args) {
    // 错误处理...
    return;
  }
  // 更新接收区写指针 - args是当前DMA传输的缓冲区位置偏移(Circular
  // Mode)或传输量(Normal Mode) 假设使用 ReceiveToIdle + Circular Mode
  size_t current_pos = (size_t)args;
  size_t len_to_advance = 0;

  if (current_pos >= queue->last_rx_pos) {
    len_to_advance = current_pos - queue->last_rx_pos;
  } else {
    // 发生了回绕
    len_to_advance = (queue->rx_rb.size - queue->last_rx_pos) + current_pos;
  }

  if (len_to_advance > 0) {
    rb_advance_head(&queue->rx_rb, len_to_advance);
    queue->last_rx_pos = current_pos;
  }

#if Rx_Manual_restart
  // 重新启动接收
  if (queue->rx_enabled) {
    uint8_t *rx_ptr;
    size_t available_len = rb_peek_write(&queue->rx_rb, &rx_ptr);
    if (available_len > 0) {
      USART_RECEIVE_ASYN((usart_driver_t *)queue->uart_driver, rx_ptr,
                         available_len);
    }
  }
#endif
}
