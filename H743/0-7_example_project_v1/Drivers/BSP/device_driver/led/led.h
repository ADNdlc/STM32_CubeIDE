/*
 * led.h
 *
 *  Created on: Nov 25, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_LED_LED_H_
#define BSP_DEVICE_DRIVER_LED_LED_H_

typedef struct led_t led_t;// 前向声明

led_t *led_create(GPIO_TypeDef *port, uint16_t pin);
void led_delete(led_t *self);

#endif /* BSP_DEVICE_DRIVER_LED_LED_H_ */
