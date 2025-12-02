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
  usart_driver_t base;
  UART_HandleTypeDef *huart;
} stm32_usart_driver_t;

stm32_usart_driver_t *stm32_usart_driver_create(UART_HandleTypeDef *huart);

#endif /* BSP_DEVICE_DRIVER_PLATFORM_STM32_DEVICE_STM32_USART_DRIVER_H_ */
