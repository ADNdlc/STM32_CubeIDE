/*
 * gpio_led.h
 *
 *  Created on: Mar 22, 2026
 *      Author: 12114
 */

#ifndef GPIO_LED_GPIO_LED_H_
#define GPIO_LED_GPIO_LED_H_

#include "gpio_driver.h"
#include <stdint.h>

typedef enum {
  GPIO_LED_OFF = 0,
  GPIO_LED_ON,
} gpio_led_state_t;

typedef struct {
  gpio_driver_t *gpio;
  gpio_led_state_t active_level;
} gpio_led_t;

void gpio_led_init(gpio_led_t *led, gpio_driver_t *gpio, uint8_t active_level);
gpio_led_t *gpio_led_create(gpio_driver_t *driver, uint8_t active_level);
void gpio_led_destroy(gpio_led_t *led);

void gpio_led_set(gpio_led_t *led, gpio_led_state_t new_state);
gpio_led_state_t gpio_led_get(gpio_led_t *led);
void gpio_led_toggle(gpio_led_t *led);

#endif /* GPIO_LED_GPIO_LED_H_ */
