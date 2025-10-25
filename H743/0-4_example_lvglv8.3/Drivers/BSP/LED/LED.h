/*
 * LED.h
 *
 *  Created on: Aug 21, 2025
 *      Author: 12114
 */

#ifndef BSP_LED_LED_H_
#define BSP_LED_LED_H_

#include "GPIO.h"

#define LED_Pin		GPIO_PIN_1
#define LED_Port	GPIOB

#define LED_ON		HAL_GPIO_WritePin(LED_Port,LED_Pin,GPIO_PIN_RESET);
#define LED_OFF		HAL_GPIO_WritePin(LED_Port,LED_Pin,GPIO_PIN_SET);
#define LED_SET(x)	(X)?LED_ON:LED_OFF
#define LED_READ	HAL_GPIO_ReadPin(LED_Port,LED_Pin);
#define LED_TOGGLE	HAL_GPIO_TogglePin(LED_Port,LED_Pin);

void led_control_init(void);

#endif /* BSP_LED_LED_H_ */
