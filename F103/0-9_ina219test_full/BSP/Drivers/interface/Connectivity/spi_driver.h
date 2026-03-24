/*
 * spi_driver.h
 *
 *  Created on: Feb 16, 2026
 *      Author: Antigravity
 */

#ifndef BSP_DEVICE_DRIVER_INTERFACE_SPI_DRIVER_H_
#define BSP_DEVICE_DRIVER_INTERFACE_SPI_DRIVER_H_

#include <stddef.h>
#include <stdint.h>

// 前向声明
typedef struct spi_driver_t spi_driver_t;

// SPI 驱动操作接口 (虚函数表)
typedef struct {
  // 发送数据
  int (*transmit)(spi_driver_t *self, const uint8_t *data, uint16_t size,
                  uint32_t timeout);
  // 接收数据
  int (*receive)(spi_driver_t *self, uint8_t *buffer, uint16_t size,
                 uint32_t timeout);
  // 全双工收发
  int (*transmit_receive)(spi_driver_t *self, const uint8_t *tx_data,
                          uint8_t *rx_buffer, uint16_t size, uint32_t timeout);
} spi_driver_ops_t;

// SPI 驱动基类
struct spi_driver_t {
  const spi_driver_ops_t *ops;
};

// 辅助宏
#define SPI_TRANSMIT(driver, data, size, timeout)                              \
  (driver)->ops->transmit(driver, data, size, timeout)

#define SPI_RECEIVE(driver, buffer, size, timeout)                             \
  (driver)->ops->receive(driver, buffer, size, timeout)

#define SPI_TRANSMIT_RECEIVE(driver, tx_data, rx_buffer, size, timeout)        \
  (driver)->ops->transmit_receive(driver, tx_data, rx_buffer, size, timeout)

#endif /* BSP_DEVICE_DRIVER_INTERFACE_SPI_DRIVER_H_ */
