/*
 * ring_buffer.c
 *
 *  Created on: Dec 1, 2025
 *      Author: 12114
 */

#include "ring_buffer.h"
#include <stdlib.h>
#include <string.h>

#define RB_MIN(a, b) ((a) < (b) ? (a) : (b))

ring_buffer_t* rb_create(uint8_t *buffer, size_t size) {
    ring_buffer_t *rb = (ring_buffer_t *)malloc(sizeof(ring_buffer_t));
    if (!rb) return NULL;
    rb_init(rb, buffer, size);
    return rb;
}

void rb_destroy(ring_buffer_t *rb) {
  if (rb) {
    free(rb);
  }
}

void rb_init(ring_buffer_t *rb, uint8_t *buffer, size_t size) {
  rb->buffer = buffer;
  rb->size = size;
  rb->head = rb->tail = 0;
}

bool rb_push(ring_buffer_t *rb, uint8_t data) {
  size_t next = (rb->head + 1) % rb->size;
  if (next == rb->tail)
    return false;
  rb->buffer[rb->head] = data;
  rb->head = next;
  return true;
}

// 获取一段连续的可读内存，方便DMA直接搬运
size_t rb_peek(ring_buffer_t *rb, uint8_t **ptr) {
  if (rb->head == rb->tail)
    return 0;
  *ptr = &rb->buffer[rb->tail];
  if (rb->head > rb->tail)
    return rb->head - rb->tail;
  return rb->size - rb->tail; // 到缓冲区末尾的长度
}

void rb_advance(ring_buffer_t *rb, size_t len) {
  rb->tail = (rb->tail + len) % rb->size;
}

size_t rb_available(ring_buffer_t *rb) {
  if (rb->head >= rb->tail)
    return rb->head - rb->tail;
  return rb->size + rb->head - rb->tail;
}

size_t rb_free_space(ring_buffer_t *rb) {
  if (rb->tail > rb->head)
    return rb->tail - rb->head - 1;
  return rb->size - 1 - (rb->head - rb->tail);
}

size_t rb_read(ring_buffer_t *rb, uint8_t *data, size_t len) {
  size_t available = rb_available(rb);
  if (len > available)
    len = available;

  size_t to_end = rb->size - rb->tail;
  if (len > to_end) {
    memcpy(data, &rb->buffer[rb->tail], to_end);
    memcpy(data + to_end, &rb->buffer[0], len - to_end);
  } else {
    memcpy(data, &rb->buffer[rb->tail], len);
  }
  rb->tail = (rb->tail + len) % rb->size;
  return len;
}

size_t rb_write(ring_buffer_t *rb, const uint8_t *data, size_t len) {
  size_t free = rb_free_space(rb);
  if (len > free)
    len = free;

  size_t to_end = rb->size - rb->head;
  if (len > to_end) {
    memcpy(&rb->buffer[rb->head], data, to_end);
    memcpy(&rb->buffer[0], data + to_end, len - to_end);
  } else {
    memcpy(&rb->buffer[rb->head], data, len);
  }
  rb->head = (rb->head + len) % rb->size;
  return len;
}

size_t rb_peek_write(ring_buffer_t *rb, uint8_t **ptr) {
  size_t free = rb_free_space(rb);
  if (free == 0)
    return 0;

  *ptr = &rb->buffer[rb->head];
  size_t to_end = rb->size - rb->head;
  return (free < to_end) ? free : to_end;
}

void rb_advance_head(ring_buffer_t *rb, size_t len) {
  rb->head = (rb->head + len) % rb->size;
}
