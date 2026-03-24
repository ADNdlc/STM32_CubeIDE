/*
 * ring_buffer.c
 *
 *  Created on: Dec 1, 2025
 *      Author: 12114
 */

#include "ring_buffer.h"
#include <stdlib.h>
#include <string.h>
#include "sys.h"
#include "MemPool.h"
#ifdef USE_MEMPOOL
//从哪里分配
#define RINGBUF_MEMSOURCE SYS_MEM_INTERNAL
#endif
/**
 * @brief 获取两个值中的较小值
 */
#define RB_MIN(a, b) ((a) < (b) ? (a) : (b))

/**
 * @brief 创建并初始化一个环形缓冲区对象
 * @param buffer 缓冲区内存指针
 * @param size 缓冲区大小
 * @return 成功返回环形缓冲区对象指针，失败返回NULL
 */
ring_buffer_t* rb_create(uint8_t *buffer, size_t size) {
#ifdef USE_MEMPOOL
    ring_buffer_t *rb = (ring_buffer_t *)sys_malloc(RINGBUF_MEMSOURCE, sizeof(ring_buffer_t));
#else
    ring_buffer_t *rb = (ring_buffer_t *)malloc(sizeof(ring_buffer_t));

#endif
    if (!rb) return NULL;
    rb_init(rb, buffer, size);
    return rb;
}

/**
 * @brief 销毁环形缓冲区对象
 * @param rb 环形缓冲区对象指针
 */
void rb_destroy(ring_buffer_t *rb) {
  if (rb) {
#ifdef USE_MEMPOOL
	  sys_free(RINGBUF_MEMSOURCE, rb);
#else
	  free(rb);
#endif
  }
}

/**
 * @brief 初始化环形缓冲区
 * @param rb 环形缓冲区对象指针
 * @param buffer 缓冲区内存指针
 * @param size 缓冲区大小
 */
void rb_init(ring_buffer_t *rb, uint8_t *buffer, size_t size) {
  rb->buffer = buffer;
  rb->size = size;
  rb->head = rb->tail = 0;
}

/**
 * @brief 向环形缓冲区中压入一个字节数据
 * @param rb 环形缓冲区对象指针
 * @param data 要压入的数据
 * @return 成功返回true，缓冲区满返回false
 */
bool rb_push(ring_buffer_t *rb, uint8_t data) {
  size_t next = (rb->head + 1) % rb->size;
  if (next == rb->tail)
    return false;
  rb->buffer[rb->head] = data;
  rb->head = next;
  return true;
}

/**
 * @brief 获取一段连续的可读内存，方便DMA直接搬运
 * @param rb 环形缓冲区对象指针
 * @param ptr 指向可读数据起始地址的指针
 * @return 连续可读数据的长度
 * @note 此函数不移动读指针，需要配合rb_advance使用
 */
size_t rb_peek(ring_buffer_t *rb, uint8_t **ptr) {
  if (rb->head == rb->tail)
    return 0;
  *ptr = &rb->buffer[rb->tail];
  if (rb->head > rb->tail)
    return rb->head - rb->tail;
  return rb->size - rb->tail; // 到缓冲区末尾的长度
}

/**
 * @brief 移动读指针（用于DMA传输完成后的处理）
 * @param rb 环形缓冲区对象指针
 * @param len 需要移动的距离
 */
void rb_advance(ring_buffer_t *rb, size_t len) {
  rb->tail = (rb->tail + len) % rb->size;
}

/**
 * @brief 获取环形缓冲区中可读数据的字节数
 * @param rb 环形缓冲区对象指针
 * @return 可读数据的字节数
 */
size_t rb_available(ring_buffer_t *rb) {
  if (rb->head >= rb->tail)
    return rb->head - rb->tail;
  return rb->size + rb->head - rb->tail;
}

/**
 * @brief 获取环形缓冲区中空闲空间的字节数
 * @param rb 环形缓冲区对象指针
 * @return 空闲空间的字节数
 */
size_t rb_free_space(ring_buffer_t *rb) {
  if (rb->tail > rb->head)
    return rb->tail - rb->head - 1;
  return rb->size - 1 - (rb->head - rb->tail);
}

/**
 * @brief 从环形缓冲区中读取数据
 * @param rb 环形缓冲区对象指针
 * @param data 存储读取数据的缓冲区
 * @param len 希望读取的数据长度
 * @return 实际读取的数据长度
 */
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

/**
 * @brief 向环形缓冲区中写入数据
 * @param rb 环形缓冲区对象指针
 * @param data 待写入数据的缓冲区
 * @param len 希望写入的数据长度
 * @return 实际写入的数据长度
 */
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

/**
 * @brief 获取一段连续的可写内存，方便DMA直接搬运
 * @param rb 环形缓冲区对象指针
 * @param ptr 指向可写空间起始地址的指针
 * @return 连续可写空间的长度
 * @note 此函数不移动写指针，需要配合rb_advance_head使用
 */
size_t rb_peek_write(ring_buffer_t *rb, uint8_t **ptr) {
  size_t free = rb_free_space(rb);
  if (free == 0)
    return 0;

  *ptr = &rb->buffer[rb->head];
  size_t to_end = rb->size - rb->head;
  return (free < to_end) ? free : to_end;
}

/**
 * @brief 移动写指针（用于DMA接收完成后的处理）
 * @param rb 环形缓冲区对象指针
 * @param len 需要移动的距离
 */
void rb_advance_head(ring_buffer_t *rb, size_t len) {
  rb->head = (rb->head + len) % rb->size;
}
