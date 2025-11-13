/*
 * LED.h
 *
 *  Created on: Aug 21, 2025
 *      Author: 12114
 */

#ifndef BSP_LED_LED_H_
#define BSP_LED_LED_H_

#include "main.h"

#define LED_Port(x) LED##x##_GPIO_Port
#define LED_Pin(x)  LED##x##_Pin

#define LED_count   2

//硬件接口定义为如下形式
// LEDx


#define LED_ON(i)		HAL_GPIO_WritePin(LED_Port(i),LED_Pin(i),GPIO_PIN_RESET)
#define LED_OFF(i)		HAL_GPIO_WritePin(LED_Port(i),LED_Pin(i),GPIO_PIN_SET)
#define LED_SET(x,i)	(x)?LED_ON(i):LED_OFF(i)
#define LED_READ(i)		HAL_GPIO_ReadPin(LED_Port(i),LED_Pin(i))
#define LED_TOGGLE(i)	HAL_GPIO_TogglePin(LED_Port(i),LED_Pin(i))


void led_control_init(void);

#endif /* BSP_LED_LED_H_ */
