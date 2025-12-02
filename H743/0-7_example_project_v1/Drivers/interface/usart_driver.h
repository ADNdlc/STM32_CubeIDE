/*
 * usart_driver.h
 *
 *  Created on: Dec 1, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_INTERFACE_USART_DRIVER_H_
#define BSP_DEVICE_DRIVER_INTERFACE_USART_DRIVER_H_

#include <stddef.h>
#include <stdint.h>


// 前向声明
typedef struct usart_driver_t usart_driver_t;

// USART 驱动操作接口 (虚函数表)
typedef struct {
  int (*transmit)(usart_driver_t *self, const uint8_t *data, size_t size);
  int (*receive)(usart_driver_t *self, uint8_t *buffer, size_t size);
  
  // 可以根据需要添加更多接口，如 set_baudrate 等
} usart_driver_ops_t;

// USART 驱动基类
struct usart_driver_t {
  const usart_driver_ops_t *ops;
};

// 辅助宏
#define USART_TRANSMIT(driver, data, size)                                     \
  (driver)->ops->transmit(driver, data, size)
#define USART_RECEIVE(driver, buffer, size)                                    \
  (driver)->ops->receive(driver, buffer, size)

#endif /* BSP_DEVICE_DRIVER_INTERFACE_USART_DRIVER_H_ */
