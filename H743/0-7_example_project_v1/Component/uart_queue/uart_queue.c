/*
 * uart_queue.c
 *
 *  Created on: Dec 1, 2025
 *      Author: 12114
 */

#include "async_uart.h"

void async_uart_init(async_uart_t *uart, const uart_ops_t *ops,
                     uint8_t *tx_buf, size_t tx_size,
                     uint8_t *rx_buf, size_t rx_size,
                     void *user_data) {
    rb_init(&uart->tx_rb, tx_buf, tx_size);
    rb_init(&uart->rx_rb, rx_buf, rx_size);
    uart->ops = ops;
    uart->tx_busy = false;
    uart->user_data = user_data;
}

// 内部函数：尝试触发下一次传输
static void _try_transmit(async_uart_t *uart) {
    uint8_t *ptr;
    // 获取缓冲区中连续的数据块
    size_t len = rb_peek(&uart->tx_rb, &ptr);

    if (len > 0) {
        uart->tx_busy = true;
        // 调用底层DMA发送
        uart->ops->transmit_dma(uart, ptr, len);
    } else {
        uart->tx_busy = false;
    }
}

size_t async_uart_write(async_uart_t *uart, const uint8_t *data, size_t len) {
    size_t written = 0;

    uart->ops->enter_critical();

    for (size_t i = 0; i < len; i++) {
        if (!rb_push(&uart->tx_rb, data[i])) {
            break; // 缓冲区满
        }
        written++;
    }

    // 如果当前硬件空闲，立即触发发送
    if (!uart->tx_busy && written > 0) {
        _try_transmit(uart);
    }

    uart->ops->exit_critical();
    return written;
}

// 接收读取
size_t async_uart_read(async_uart_t *uart, uint8_t *data, size_t len) {
    size_t read_len = 0;
    uart->ops->enter_critical();
    // 简单的从RB读取
    for(size_t i=0; i<len; i++) {
        /* 此处简化调用，实际可优化为块读取 */
        // rb_pop ...
    }
    uart->ops->exit_critical();
    return read_len;
}

// --- 硬件回调区 ---

// DMA发送完成中断中调用此函数
void async_uart_isr_tx_cplt(async_uart_t *uart) {
    uint8_t *ptr;
    size_t len = rb_peek(&uart->tx_rb, &ptr);

    // 我们刚刚发送完了 len 长度的数据，现在把它们从环形缓冲中移除
    rb_advance(&uart->tx_rb, len);

    // 检查是否还有剩余数据需要发送（环形缓冲回卷的情况）
    _try_transmit(uart);
}

// 接收中断调用
void async_uart_isr_rx_data(async_uart_t *uart, uint8_t data) {
    rb_push(&uart->rx_rb, data);
}
