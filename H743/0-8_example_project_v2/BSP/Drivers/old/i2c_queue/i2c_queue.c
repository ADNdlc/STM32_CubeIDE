/*
 * i2c_queue.c
 *
 *  Created on: Feb 19, 2026
 *      Author: Antigravity
 */

#include "i2c_queue.h"
#include <stdlib.h>
#include <string.h>


/**
 * @brief I2C回调函数
 */
static void i2c_queue_callback(void *context, i2c_event_t event, void *args) {
  i2c_queue_t *queue = (i2c_queue_t *)context;

  switch (event) {
  case I2C_EVENT_RX_COMPLETE:
    // DMA 接收完成后，数据已经直接进入了 ring buffer 的缓冲区（由 start_receive
    // 指定） 我们需要更新 ring buffer 的 head 指针
    if (args != NULL) {
      size_t received_len = (size_t)args;
      rb_advance_head(&queue->rx_rb, received_len);
    }
    queue->rx_busy = false;
    // 重新启动接收以维持连续性 (非循环 DMA 模式下需要手动重启)
    i2c_queue_start_receive(queue);
    break;

  case I2C_EVENT_ERROR:
    queue->rx_busy = false;
    // 发生错误时尝试重新启动
    i2c_queue_start_receive(queue);
    break;

  default:
    break;
  }
}

void i2c_queue_init(i2c_queue_t *queue, const i2c_driver_t *driver,
                    uint16_t dev_addr, uint8_t *rx_buffer, size_t rx_size) {
  if (!queue || !driver || !rx_buffer)
    return;

  rb_init(&queue->rx_rb, rx_buffer, rx_size);
  queue->i2c_driver = driver;
  queue->dev_addr = dev_addr;
  queue->rx_enabled = false;
  queue->rx_busy = false;

  // 注册回调
  I2C_SET_CALLBACK((i2c_driver_t *)driver, i2c_queue_callback, queue);
}

int i2c_queue_start_receive(i2c_queue_t *queue) {
  if (!queue || !queue->i2c_driver)
    return -1;

  queue->rx_enabled = true;

  if (!queue->rx_busy) {
    uint8_t *rx_ptr;
    size_t available_len = rb_peek_write(&queue->rx_rb, &rx_ptr);

    if (available_len > 0) {
      int ret = I2C_MASTER_RECEIVE_ASYN((i2c_driver_t *)queue->i2c_driver,
                                        queue->dev_addr, rx_ptr, available_len);
      if (ret == 0) {
        queue->rx_busy = true;
        return 0;
      } else {
        return -1; // 不支持或启动失败
      }
    }
  }
  return 0;
}

size_t i2c_queue_getdata(i2c_queue_t *queue, uint8_t *data, size_t max_len) {
  if (!queue)
    return 0;
  return rb_read(&queue->rx_rb, data, max_len);
}

size_t i2c_queue_rx_count(i2c_queue_t *queue) {
  if (!queue)
    return 0;
  return rb_available(&queue->rx_rb);
}
