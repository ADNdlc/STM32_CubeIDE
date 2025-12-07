/*
 * device_mapping.h
 *
 *  Created on: Nov 28, 2025
 *      Author: 12114
 */

#ifndef STM32H743_BOARD_V1_DEVICE_MAPPING_H_
#define STM32H743_BOARD_V1_DEVICE_MAPPING_H_

#include "stm32h7xx_hal.h"

// GPIO 设备逻辑标识枚举
typedef enum
{
  GPIO_LED_0 = 0,
  GPIO_BUTTON_KEYUP,
  GPIO_BUTTON_KEY0,
  GPIO_BUTTON_KEY1,
  GPIO_BUTTON_KEY2,
  //...
  GPIO_MAX_DEVICES
} gpio_device_id_t;

// PWM 设备逻辑标识枚举
typedef enum
{
  RGB_LED_RED = 0,
  RGB_LED_GREEN,
  RGB_LED_BLUE,
  PWM_LED_1,
  //...
  PWM_MAX_DEVICES
} pwm_device_id_t;

// USART 设备逻辑标识枚举
typedef enum
{
  USART_LOGGER = 0,
  USART_ATCMD,
  //...
  USART_MAX_DEVICES
} usart_device_id_t;

// GPIO 设备映射结构体
typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
} gpio_mapping_t;

// PWM 设备映射结构体
typedef struct
{
  TIM_HandleTypeDef *htim;
  uint32_t channel;
} pwm_mapping_t;

// USART 设备映射结构体
typedef struct
{
  UART_HandleTypeDef *huart;
} usart_mapping_t;

// 导出映射表
extern const gpio_mapping_t gpio_mappings[GPIO_MAX_DEVICES];
extern const pwm_mapping_t pwm_mappings[PWM_MAX_DEVICES];
extern const usart_mapping_t usart_mappings[USART_MAX_DEVICES];

#endif /* STM32H743_BOARD_V1_DEVICE_MAPPING_H_ */
