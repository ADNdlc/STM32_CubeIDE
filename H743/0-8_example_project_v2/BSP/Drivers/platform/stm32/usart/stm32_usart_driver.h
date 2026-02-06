/*
 * stm32_usart_driver.h
 *
 *  Created on: Dec 1, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_PLATFORM_STM32_DEVICE_STM32_USART_DRIVER_H_
#define BSP_DEVICE_DRIVER_PLATFORM_STM32_DEVICE_STM32_USART_DRIVER_H_

#include "stm32h7xx_hal.h"
#include "usart_driver.h"

typedef struct {
  usart_driver_t base;	// 继承后扩展到平台实现
  UART_HandleTypeDef *huart;

  // 回调函数和上下文
  usart_callback_t callback;
  void *cb_context; // 用户上下文
  uint8_t T_isbusy;
  uint8_t R_isbusy;
} stm32_usart_driver_t;

stm32_usart_driver_t *stm32_usart_driver_create(UART_HandleTypeDef *huart);

#endif /* BSP_DEVICE_DRIVER_PLATFORM_STM32_DEVICE_STM32_USART_DRIVER_H_ */
