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



void LED_init(void);
uint8_t LED_Get_States(void);
void LED_MQTT_publish(void);

#endif /* BSP_LED_LED_H_ */
