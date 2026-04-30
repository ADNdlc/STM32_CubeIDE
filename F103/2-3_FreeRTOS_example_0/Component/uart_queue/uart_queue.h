/*
 * uart_queue.h
 *
 *  Created on: Dec 3, 2025
 *      Author: 12114
 */
#ifndef UART_QUEUE_UART_QUEUE_H_
#define UART_QUEUE_UART_QUEUE_H_

#include "interface_inc.h"
#include "Sys.h"
#include "ring_buffer/ring_buffer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define UART_QUEUE_AUTO_WAIT_DELAY_MS 5   // 每次自动等待时的阻塞时间（毫秒）
#define UART_QUEUE_AUTO_WAIT_MAX_COUNT 20 // 自动等待最大次数

// 前向声明
typedef struct uart_queue_t uart_queue_t;

/**
 * @brief UART队列结构体
 */
struct uart_queue_t {
  ring_buffer_t tx_rb;               // 发送环形缓冲
  ring_buffer_t rx_rb;               // 接收环形缓冲
  const usart_driver_t *uart_driver; // 硬件驱动接口
  bool tx_busy;                      // 发送忙标志
  bool rx_enabled;                   // 接收使能标志
  size_t tx_current_len;             // 当前发送长度
  size_t last_rx_pos;                // 记录上一次接收位置(针对循环DMA)
  bool auto_wait;                    // 是否启用发送缓冲区满自动等待
  uint32_t wait_delay_ms;            // 每次等待的时间
  uint16_t wait_max_count;           // 最大等待次数
};

uart_queue_t *uart_queue_create(const usart_driver_t *driver,
                                uint8_t *tx_buffer, size_t tx_size,
                                uint8_t *rx_buffer, size_t rx_size);

void uart_queue_destroy(uart_queue_t *queue);

// 初始化
void uart_queue_init(uart_queue_t *queue, const usart_driver_t *driver,
                     uint8_t *tx_buffer, size_t tx_size, uint8_t *rx_buffer,
                     size_t rx_size);

// 发送数据到队列
bool uart_queue_send(uart_queue_t *queue, const uint8_t *data, size_t len);

// 设置是否启用发送缓冲区满自动等待
void uart_queue_set_auto_wait(uart_queue_t *queue, bool enabled);

// 设置自动等待的参数
void uart_queue_set_wait_config(uart_queue_t *queue, uint32_t delay_ms,
                                uint16_t max_count);

// 从接收队列读取数据
size_t uart_queue_getdata(uart_queue_t *queue, uint8_t *data, size_t max_len);

// 获取发送队列中可用数据的数量
size_t uart_queue_tx_count(uart_queue_t *queue);

// 获取接收队列中可用数据的数量
size_t uart_queue_rx_count(uart_queue_t *queue);

// 启动接收
void uart_queue_start_receive(uart_queue_t *queue);

#endif /* UART_QUEUE_UART_QUEUE_H_ */
