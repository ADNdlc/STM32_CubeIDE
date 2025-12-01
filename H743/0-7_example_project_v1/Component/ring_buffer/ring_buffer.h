/*
 * ring_buffer.h
 *
 *  Created on: Dec 1, 2025
 *      Author: 12114
 */

#ifndef UART_QUEUE_RING_BUFFER_H_
#define UART_QUEUE_RING_BUFFER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// 环形缓冲区
typedef struct {
    uint8_t *buffer;
    size_t size;
    volatile size_t head; // 写入位置
    volatile size_t tail; // 读取位置
} ring_buffer_t;

void rb_init(ring_buffer_t *rb, uint8_t *buffer, size_t size);
bool rb_push(ring_buffer_t *rb, uint8_t data);
bool rb_pop(ring_buffer_t *rb, uint8_t *data);
size_t rb_read(ring_buffer_t *rb, uint8_t *data, size_t len);
size_t rb_peek(ring_buffer_t *rb, uint8_t **ptr); // 获取连续数据的指针（用于DMA）
void rb_advance(ring_buffer_t *rb, size_t len);   // 手动移动读指针
size_t rb_available(ring_buffer_t *rb);

#endif /* UART_QUEUE_RING_BUFFER_H_ */
