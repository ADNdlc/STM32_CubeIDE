/*
 * uart_queue.c
 *
 *  Created on: Dec 3, 2025
 *      Author: 12114
 */

#include "uart_queue.h"
#include "stm32h7xx_hal.h"

#define restart 0

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

/**
 * @brief 初始化UART队列
 *
 * @param queue 队列实例
 * @param hal USART硬件抽象
 * @param tx_buffer 发送缓冲区
 * @param tx_size 发送缓冲区大小
 * @param rx_buffer 接收缓冲区
 * @param rx_size 接收缓冲区大小
 */
void uart_queue_init(uart_queue_t *queue, const usart_hal_t *hal,
                     uint8_t *tx_buffer, size_t tx_size, uint8_t *rx_buffer,
                     size_t rx_size) {
  rb_init(&queue->tx_rb, tx_buffer, tx_size);
  rb_init(&queue->rx_rb, rx_buffer, rx_size);
  queue->uart_hal = hal;
  queue->tx_busy = false;
  queue->rx_enabled = false;
  queue->tx_current_len = 0;
  // 设置回调函数
  usart_hal_set_callback((usart_hal_t *)queue->uart_hal, uart_queue_callback,
                         queue);
}

uart_queue_t *uart_queue_create(const usart_hal_t *hal, uint8_t *tx_buffer,
                                size_t tx_size, uint8_t *rx_buffer,
                                size_t rx_size) {
  uart_queue_t *queue = (uart_queue_t *)malloc(sizeof(uart_queue_t));
  if (!queue) {
    return;
  }
  uart_queue_init(queue, hal, tx_buffer, tx_size, rx_buffer, rx_size);
  return queue;
}

void uart_queue_destroy(uart_queue_t *queue) {
  if (queue) {
    free(queue);
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
 * @brief 发送数据到队列
 *
 * @param queue 队列实例
 * @param data 要发送的数据
 * @param len 数据长度
 * @return bool 是否成功加入队列
 */
bool uart_queue_send(uart_queue_t *queue, const uint8_t *data, size_t len) {
  size_t written = rb_write(&queue->tx_rb, data, len);

  // 如果有数据写入且当前没有在发送，则启动发送
  if (written > 0 && !queue->tx_busy) {
    uint8_t *tx_ptr;
    size_t tx_len = rb_peek(&queue->tx_rb, &tx_ptr);
    if (tx_len > 0) {
      queue->tx_busy = true;
      queue->tx_current_len = tx_len;
      if (usart_hal_transmit_asyn((usart_hal_t *)queue->uart_hal, tx_ptr,
                                  tx_len) != 0) {
        queue->tx_busy = false;
        queue->tx_current_len = 0;
      }
    }
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

    // 启动异步接收
    uint8_t *rx_ptr;
    size_t rx_len = rb_peek_write(&queue->rx_rb, &rx_ptr);
    if (rx_len > 0) {
      usart_hal_receive_asyn((usart_hal_t *)queue->uart_hal, rx_ptr, rx_len);
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
    usart_hal_transmit_asyn((usart_hal_t *)queue->uart_hal, tx_ptr, tx_len);
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
      usart_hal_receive_asyn((usart_hal_t *)queue->uart_hal, rx_ptr,
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
  // 更新接收区写指针 - args是接收到的数据长度
  size_t rx_len = (size_t)args;
  rb_advance_head(&queue->rx_rb, rx_len);

#if restart
  // 重新启动接收
  if (queue->rx_enabled) {
    uint8_t *rx_ptr;
    size_t available_len = rb_peek_write(&queue->rx_rb, &rx_ptr);
    if (available_len > 0) {
      usart_hal_receive_asyn((usart_hal_t *)queue->uart_hal, rx_ptr,
                             available_len);
    }
  }
#endif
}
