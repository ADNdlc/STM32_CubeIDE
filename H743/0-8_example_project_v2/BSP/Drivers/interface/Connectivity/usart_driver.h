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

// 事件类型定义
typedef enum{
  USART_EVENT_RX_COMPLETE, // 接收到数据
  USART_EVENT_TX_COMPLETE, // 发送完成
  USART_EVENT_ERROR,       // 发生错误
  USART_EVENT_RX_EVENT,
  //...
} usart_event_t;

// 回调函数原型
// context: 上下文
// event: 事件类型
// args: 事件参数(如接收到的数据长度等)
typedef void (*usart_callback_t)(void *context, usart_event_t event, void *args);

// USART 驱动操作接口 (虚函数表)
typedef struct{
  int (*transmit)(usart_driver_t *self, const uint8_t *data, size_t size,
                  uint32_t timeout);
  int (*receive)(usart_driver_t *self, uint8_t *buffer, size_t size,
                 uint32_t timeout);
  int (*transmit_asyn)(usart_driver_t *self, const uint8_t *data, size_t size);
  int (*receive_asyn)(usart_driver_t *self, uint8_t *buffer, size_t size);

  // 注册回调函数
  int (*set_callback)(usart_driver_t *self, // 驱动实例
                      usart_callback_t cb,  // 回调函数
                      void *cb_context);    // 上下文(对象指针)
} usart_driver_ops_t;

// USART 驱动基类
struct usart_driver_t{
  const usart_driver_ops_t *ops;
};

// 辅助宏
#define USART_TRANSMIT(driver, data, size, timeout) \
  (driver)->ops->transmit(driver, data, size, timeout)
#define USART_RECEIVE(driver, buffer, size, timeout) \
  (driver)->ops->receive(driver, buffer, size, timeout)
#define USART_TRANSMIT_ASYN(driver, data, size) \
  (driver)->ops->transmit_asyn(driver, data, size)
#define USART_RECEIVE_ASYN(driver, buffer, size) \
  (driver)->ops->receive_asyn(driver, buffer, size)
#define USART_SET_CALLBACK(driver, cb, ctx) \
  (driver)->ops->set_callback(driver, cb, ctx)

#endif /* BSP_DEVICE_DRIVER_INTERFACE_USART_DRIVER_H_ */
