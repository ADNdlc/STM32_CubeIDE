/*
 * uart_queue.h
 *
 *  Created on: Dec 1, 2025
 *      Author: 12114
 */

#ifndef UART_QUEUE_UART_QUEUE_H_
#define UART_QUEUE_UART_QUEUE_H_

#ifndef ASYNC_UART_H
#define ASYNC_UART_H

#include "ring_buffer.h"

// 前向声明
typedef struct async_uart_s async_uart_t;

/**
 * @brief 硬件抽象接口 (依赖注入的核心)
 */
typedef struct {
    // 启动DMA发送，addr为数据首地址，len为长度
    int (*transmit_dma)(async_uart_t *ctx, uint8_t *addr, size_t len);
    // 进入临界区（关中断）
    void (*enter_critical)(void);
    // 退出临界区（开中断）
    void (*exit_critical)(void);
} uart_ops_t;

/**
 * @brief 异步UART控制块
 */
struct async_uart_s {
    ring_buffer_t tx_rb;      // 发送环形缓冲
    ring_buffer_t rx_rb;      // 接收环形缓冲
    const uart_ops_t *ops;    // 硬件操作接口
    volatile bool tx_busy;    // 发送忙标志
    void *user_data;          // 用户自定义数据（如HAL句柄）
};

// 初始化
void async_uart_init(async_uart_t *uart, const uart_ops_t *ops,
                     uint8_t *tx_buf, size_t tx_size,
                     uint8_t *rx_buf, size_t rx_size,
                     void *user_data);

// 用户API：发送数据（非阻塞，写入缓冲即返回）
size_t async_uart_write(async_uart_t *uart, const uint8_t *data, size_t len);

// 用户API：读取数据
size_t async_uart_read(async_uart_t *uart, uint8_t *data, size_t len);

// 硬件驱动需要调用的回调：当DMA发送完成时
void async_uart_isr_tx_cplt(async_uart_t *uart);

// 硬件驱动需要调用的回调：当收到数据时 (字节中断或DMA空闲中断)
void async_uart_isr_rx_data(async_uart_t *uart, uint8_t data);
// 或者如果是DMA接收，可以直接操作rx_rb的head指针，这里为了简单演示字节接收

#endif /* UART_QUEUE_UART_QUEUE_H_ */
