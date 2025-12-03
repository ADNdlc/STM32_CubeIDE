/*
 * uart_queue.h
 *
 *  Created on: Dec 3, 2025
 *      Author: 12114
 */

#ifndef UART_QUEUE_UART_QUEUE_H_
#define UART_QUEUE_UART_QUEUE_H_

#include "ring_buffer.h"
#include "usart_hal.h"

// 前向声明
typedef struct uart_queue_t uart_queue_t;

/**
 * @brief UART队列结构体
 */
struct uart_queue_t { 
    ring_buffer_t tx_rb;      // 发送环形缓冲
    ring_buffer_t rx_rb;      // 接收环形缓冲
    const usart_hal_t *hal;   // 硬件抽象层

};

// 初始化
void uart_queue_init();



#endif /* UART_QUEUE_UART_QUEUE_H_ */
