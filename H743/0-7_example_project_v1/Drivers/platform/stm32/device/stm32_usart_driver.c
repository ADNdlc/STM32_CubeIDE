/*
 * stm32_usart_driver.c
 *
 *  Created on: Dec 1, 2025
 *      Author: 12114
 */

#include "stm32_usart_driver.h"
#include <stdlib.h>

#define MAX_USART_INSTANCES 8
static stm32_usart_driver_t *g_usart_instances[MAX_USART_INSTANCES] = {0};

// 实例注册
static void register_instance(stm32_usart_driver_t *instance) {
  for (int i = 0; i < MAX_USART_INSTANCES; i++) {
    if (g_usart_instances[i] == NULL) {
      g_usart_instances[i] = instance;
      return;
    }
  }
}

// 为实例绑定回调
static int stm32_usart_set_callback(usart_driver_t *base, usart_callback_t cb,
                                    void *ctx) {
  stm32_usart_driver_t *self = (stm32_usart_driver_t *)base;
  self->callback = cb;
  self->cb_context = ctx;
  return 0;
}

// 同步发送函数
static int stm32_usart_transmit(usart_driver_t *self, const uint8_t *data,
                                size_t size, uint32_t timeout) {
  stm32_usart_driver_t *driver = (stm32_usart_driver_t *)self;
  
  // 使用阻塞模式发送
  if (HAL_UART_Transmit(driver->huart, (uint8_t *)data, (uint16_t)size,
                        timeout) == HAL_OK) {
    return 0;
  }
  return -1;
}

// 同步接收函数
static int stm32_usart_receive(usart_driver_t *self, uint8_t *buffer,
                               size_t size, uint32_t timeout) {
  stm32_usart_driver_t *driver = (stm32_usart_driver_t *)self;

  // 使用阻塞模式接收
  if (HAL_UART_Receive(driver->huart, buffer, (uint16_t)size, timeout) ==
      HAL_OK) {
    return 0;
  }
  return -1;
}

// 异步发送函数（使用DMA）
static int stm32_usart_transmit_asyn(usart_driver_t *self, const uint8_t *data,
                                     size_t size) {
  stm32_usart_driver_t *driver = (stm32_usart_driver_t *)self;
  if (driver->T_isbusy) {
    return -1;
  }
  driver->T_isbusy = 1;

  if (driver->callback == NULL) {
    return -1;
  }

  // 使用DMA模式发送
  if (HAL_UART_Transmit_DMA(driver->huart, (uint8_t *)data, (uint16_t)size) ==
      HAL_OK) {
    return 0;
  }
  return -1;
}

// 异步接收函数（使用DMA）
static int stm32_usart_receive_asyn(usart_driver_t *self, uint8_t *buffer,
                                    size_t size) {
  stm32_usart_driver_t *driver = (stm32_usart_driver_t *)self;
  if (driver->R_isbusy) {
    return -1;
  }
  driver->R_isbusy = 1;

  if (driver->callback == NULL) {
    return -1;
  }

  // 使用DMA模式接收
  if (HAL_UART_Receive_DMA(driver->huart, buffer, (uint16_t)size) == HAL_OK) {
    return 0;
  }
  return -1;
}

// stm32 USART 驱动类虚函数表
static const usart_driver_ops_t stm32_usart_ops = {
    .transmit = stm32_usart_transmit,
    .receive = stm32_usart_receive,
    .transmit_asyn = stm32_usart_transmit_asyn,
    .receive_asyn = stm32_usart_receive_asyn,
    .set_callback = stm32_usart_set_callback,
};

stm32_usart_driver_t *stm32_usart_driver_create(UART_HandleTypeDef *huart) {
  stm32_usart_driver_t *driver =
      (stm32_usart_driver_t *)malloc(sizeof(stm32_usart_driver_t));
  if (driver) {
    driver->base.ops = &stm32_usart_ops;
    driver->huart = huart;
    driver->callback = NULL;
    driver->cb_context = NULL;

    register_instance(driver);
  }
  return driver;
}

// 中断分发处理
static void dispatch_irq(UART_HandleTypeDef *huart, usart_event_t event) {
  for (int i = 0; i < MAX_USART_INSTANCES; i++) {
    if (g_usart_instances[i] && g_usart_instances[i]->huart == huart) {
      stm32_usart_driver_t *driver = g_usart_instances[i];
      if(event == USART_EVENT_RX_DATA){driver->R_isbusy = 0;}
      if(event == USART_EVENT_TX_COMPLETE){driver->T_isbusy = 0;}
      if (driver->callback) {
        driver->callback(driver->cb_context, event, NULL);
      }
      break;
    }
  }
}

// HAL 库回调函数重写
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  dispatch_irq(huart, USART_EVENT_RX_DATA);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  dispatch_irq(huart, USART_EVENT_TX_COMPLETE);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
  dispatch_irq(huart, USART_EVENT_ERROR);
}

// 业务层回调示例
/*
void my_uart_callback(void *context, usart_event_t event, void *args) {
    if (event == USART_EVENT_RX_DATA) { //在平台对应回调里传入的事件类型判断
        // 处理接收到的数据
    }
    else if (event == USART_EVENT_TX_COMPLETE) {
    }
    //...
}
*/
