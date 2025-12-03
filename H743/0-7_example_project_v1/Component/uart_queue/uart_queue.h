/*
 * uart_queue.h
 *
 *  Created on: Dec 3, 2025
 *      Author: 12114
 */

#ifndef UART_QUEUE_UART_QUEUE_H_
#define UART_QUEUE_UART_QUEUE_H_

#include "ring_buffer/ring_buffer.h"
#include "usart_hal/usart_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// 前向声明
typedef struct uart_queue_t uart_queue_t;

/**
 * @brief UART队列结构体
 */
struct uart_queue_t { 
    ring_buffer_t tx_rb;      // 发送环形缓冲
    ring_buffer_t rx_rb;      // 接收环形缓冲
    const usart_hal_t *uart_hal;   // 硬件抽象层
    bool tx_busy;             // 发送忙标志
    bool rx_enabled;          // 接收使能标志
};

// 初始化
void uart_queue_init(uart_queue_t *queue, const usart_hal_t *hal,
                     uint8_t *tx_buffer, size_t tx_size,
                     uint8_t *rx_buffer, size_t rx_size);

// 发送数据到队列
bool uart_queue_send(uart_queue_t *queue, const uint8_t *data, size_t len);

// 从接收队列读取数据
size_t uart_queue_getdata(uart_queue_t *queue, uint8_t *data, size_t max_len);

//获取发送队列中可用数据的数量
size_t uart_queue_tx_count(uart_queue_t *queue);

// 获取接收队列中可用数据的数量
size_t uart_queue_rx_count(uart_queue_t *queue);

// 启动接收
void uart_queue_start_receive(uart_queue_t *queue);


#endif /* UART_QUEUE_UART_QUEUE_H_ */
