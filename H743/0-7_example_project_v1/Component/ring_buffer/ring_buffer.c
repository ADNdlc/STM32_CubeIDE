/*
 * ring_buffer.c
 *
 *  Created on: Dec 1, 2025
 *      Author: 12114
 */

#include "ring_buffer.h"
#include <string.h>

#define RB_MIN(a, b) ((a) < (b) ? (a) : (b))

void rb_init(ring_buffer_t *rb, uint8_t *buffer, size_t size) {
    rb->buffer = buffer;
    rb->size = size;
    rb->head = rb->tail = 0;
}

bool rb_push(ring_buffer_t *rb, uint8_t data) {
    size_t next = (rb->head + 1) % rb->size;
    if (next == rb->tail) return false; // Full
    rb->buffer[rb->head] = data;
    rb->head = next;
    return true;
}

// 获取一段连续的可读内存，方便DMA直接搬运
size_t rb_peek(ring_buffer_t *rb, uint8_t **ptr) {
    if (rb->head == rb->tail) return 0;
    *ptr = &rb->buffer[rb->tail];
    if (rb->head > rb->tail) return rb->head - rb->tail;
    return rb->size - rb->tail; // 到缓冲区末尾的长度
}

void rb_advance(ring_buffer_t *rb, size_t len) {
    rb->tail = (rb->tail + len) % rb->size;
}

size_t rb_available(ring_buffer_t *rb) {
    if (rb->head >= rb->tail) return rb->head - rb->tail;
    return rb->size + rb->head - rb->tail;
}
