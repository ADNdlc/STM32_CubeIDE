/*
 * gpio_led.c
 *
 *  Created on: Mar 22, 2026
 *      Author: 12114
 */

#include "gpio_led.h"
#include <stdlib.h>

#include "MemPool.h"
#define GPIOLED_MEMSOURCE SYS_MEM_INTERNAL

#include "elog.h"
#define LOG_TAG "GPIO_LED"

void gpio_led_init(gpio_led_t *led, gpio_driver_t *gpio, uint8_t active_level) {
  led->gpio = gpio;
  led->active_level = active_level;

  GPIO_SET_MODE(led->gpio, GPIO_PushPullOutput);
}

gpio_led_t *gpio_led_create(gpio_driver_t *driver, uint8_t active_level) {

#ifdef USE_MEMPOOL
  gpio_led_t *led = sys_malloc(GPIOLED_MEMSOURCE, sizeof(gpio_led_t));
#else
  gpio_led_t *led = (gpio_led_t *)malloc(sizeof(gpio_led_t));
#endif

  if (led == NULL) {
    log_e("Failed to allocate memory for LED");
    return NULL;
  }
  gpio_led_init(led, driver, active_level);
  return led;
}

void gpio_led_destroy(gpio_led_t *led) {
  if (led != NULL) {
#ifdef USE_MEMPOOL
    sys_free(GPIOLED_MEMSOURCE, led);
#else
    free(led);
#endif
  }
}

void gpio_led_set(gpio_led_t *led, gpio_led_state_t new_state) {
  if (new_state == GPIO_LED_ON) {
    GPIO_WRITE(led->gpio, led->active_level);
  } else {
    GPIO_WRITE(led->gpio, !led->active_level);
  }
}

gpio_led_state_t gpio_led_get(gpio_led_t *led) { return GPIO_READ(led->gpio); }

void gpio_led_toggle(gpio_led_t *led) { GPIO_TOGGLE(led->gpio); }
