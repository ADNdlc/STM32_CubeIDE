/*
 * i2c_queue.h
 *
 *  Created on: Feb 19, 2026
 *      Author: Antigravity
 */

#ifndef COMPONENT_I2C_QUEUE_I2C_QUEUE_H_
#define COMPONENT_I2C_QUEUE_I2C_QUEUE_H_

#include "Sys.h"
#include "i2c_driver.h"
#include "ring_buffer/ring_buffer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


/**
 * @brief I2C队列结构体
 */
typedef struct {
  ring_buffer_t rx_rb;            // 接收环形缓冲
  const i2c_driver_t *i2c_driver; // 硬件驱动接口
  uint16_t dev_addr;              // 目标设备地址
  bool rx_enabled;                // 接收使能标志
  bool rx_busy;                   // 接收忙标志
} i2c_queue_t;

/**
 * @brief 初始化I2C队列
 *
 * @param queue 队列实例
 * @param driver I2C硬件驱动
 * @param dev_addr 设备地址
 * @param rx_buffer 接收缓冲区
 * @param rx_size 接收缓冲区大小
 */
void i2c_queue_init(i2c_queue_t *queue, const i2c_driver_t *driver,
                    uint16_t dev_addr, uint8_t *rx_buffer, size_t rx_size);

/**
 * @brief 启动异步接收
 * 启动后，底层DMA会自动将接收到的数据放入环形缓冲区
 *
 * @param queue 队列实例
 * @return int 0:成功, -1:底层不支持异步或失败
 */
int i2c_queue_start_receive(i2c_queue_t *queue);

/**
 * @brief 从接收队列读取数据
 *
 * @param queue 队列实例
 * @param data 存储数据的缓冲区
 * @param max_len 最大读取长度
 * @return size_t 实际读取的字节数
 */
size_t i2c_queue_getdata(i2c_queue_t *queue, uint8_t *data, size_t max_len);

/**
 * @brief 获取接收队列中可用数据的数量
 *
 * @param queue 队列实例
 * @return size_t 可用数据字节数
 */
size_t i2c_queue_rx_count(i2c_queue_t *queue);

#endif /* COMPONENT_I2C_QUEUE_I2C_QUEUE_H_ */
