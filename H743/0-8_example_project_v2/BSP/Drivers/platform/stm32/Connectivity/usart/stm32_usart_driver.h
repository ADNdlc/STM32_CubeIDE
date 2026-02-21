/*
 * stm32_usart_driver.h
 *
 *  Created on: Dec 1, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_PLATFORM_STM32_DEVICE_STM32_USART_DRIVER_H_
#define BSP_DEVICE_DRIVER_PLATFORM_STM32_DEVICE_STM32_USART_DRIVER_H_

#include "HAL_include.h"
#include "usart_driver.h"

// 从哪里分配
#define USART_MEMSOURCE SYS_MEM_INTERNAL

typedef struct {
  usart_driver_t base; // 继承后扩展到平台实现
  UART_HandleTypeDef *huart;

  // 回调函数和上下文
  usart_callback_t callback;
  void *cb_context; // 用户上下文
  uint8_t T_isbusy;
  uint8_t R_isbusy;
} stm32_usart_driver_t;

stm32_usart_driver_t *stm32_usart_driver_create(UART_HandleTypeDef *huart);

#endif /* BSP_DEVICE_DRIVER_PLATFORM_STM32_DEVICE_STM32_USART_DRIVER_H_ */
