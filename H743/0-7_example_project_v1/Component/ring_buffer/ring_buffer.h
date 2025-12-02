/*
 * ring_buffer.h
 *
 *  Created on: Dec 1, 2025
 *      Author: 12114
 */

#ifndef UART_QUEUE_RING_BUFFER_H_
#define UART_QUEUE_RING_BUFFER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// 环形缓冲区
typedef struct {
  uint8_t *buffer;
  size_t size;
  volatile size_t head; // 写入位置
  volatile size_t tail; // 读取位置
} ring_buffer_t;

ring_buffer_t* rb_create(uint8_t *buffer, size_t size);
void rb_destroy(ring_buffer_t *rb);

void rb_init(ring_buffer_t *rb, uint8_t *buffer, size_t size);
bool rb_push(ring_buffer_t *rb, uint8_t data);
bool rb_pop(ring_buffer_t *rb, uint8_t *data);
// 批量读写
size_t rb_read(ring_buffer_t *rb, uint8_t *data, size_t len);
size_t rb_write(ring_buffer_t *rb, const uint8_t *data, size_t len);

// DMA相关接口
size_t rb_peek(ring_buffer_t *rb,
               uint8_t **ptr); // 获取连续可读数据的指针(用于DMA发送)
void rb_advance(ring_buffer_t *rb,
                size_t len); // 手动移动读指针(DMA发送完成后调用)

size_t rb_peek_write(ring_buffer_t *rb,
                     uint8_t **ptr); // 获取连续可写空间的指针(用于DMA接收)
void rb_advance_head(ring_buffer_t *rb,
                     size_t len); // 手动移动写指针(DMA接收完成后调用)

size_t rb_available(ring_buffer_t *rb);  // 可读数据量
size_t rb_free_space(ring_buffer_t *rb); // 可写空间

#endif /* UART_QUEUE_RING_BUFFER_H_ */
