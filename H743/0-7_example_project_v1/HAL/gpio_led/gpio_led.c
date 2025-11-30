/*
 * gpio_led.c
 *
 *  Created on: Nov 30, 2025
 *      Author: 12114
 */

#include "gpio_led.h"
#include <stdlib.h>

// 私有函数声明
static void _gpio_led_on(led_hal_t *base);
static void _gpio_led_off(led_hal_t *base);
static void _gpio_led_set_data(led_hal_t *base, uint32_t data);
static uint32_t _gpio_led_get_data(led_hal_t *base);
static void _gpio_led_toggle(gpio_led_t *self);

// Vtable definition
static const gpio_led_vtable_t _gpio_led_vtable = {
    .base_vtable =
        {
            .on = _gpio_led_on,
            .off = _gpio_led_off,
            .set_data = _gpio_led_set_data,
            .get_data = _gpio_led_get_data,
        },
    .toggle = _gpio_led_toggle,
};

/* ==========================================
 * 构造与初始化
 * ========================================== */
void gpio_led_init(gpio_led_t *self, gpio_driver_t *driver,
                   uint8_t active_level) {
  self->base.vtable = (led_hal_vtable_t *)&_gpio_led_vtable;
  self->driver = driver;
  self->active_level = active_level;
  // Set initial state to OFF
  _gpio_led_off(&self->base);
}

gpio_led_t *gpio_led_create(gpio_driver_t *driver, uint8_t active_level) {
  gpio_led_t *led = (gpio_led_t *)malloc(sizeof(gpio_led_t));
  if (led) {
    gpio_led_init(led, driver, active_level);
  }
  return led;
}

void gpio_led_delete(gpio_led_t *self) {
  if (self) {
    free(self);
  }
}

/* ==========================================
 * 接口实现
 * ========================================== */
static void _gpio_led_on(led_hal_t *base) {
  gpio_led_t *self = (gpio_led_t *)base;
  GPIO_WRITE(self->driver, self->active_level);
}

static void _gpio_led_off(led_hal_t *base) {
  gpio_led_t *self = (gpio_led_t *)base;
  GPIO_WRITE(self->driver, !self->active_level);
}

static void _gpio_led_toggle(gpio_led_t *self) {
  uint8_t current_state = GPIO_READ(self->driver);
  GPIO_WRITE(self->driver, !current_state);
}

static void _gpio_led_set_data(led_hal_t *base, uint32_t data) {
  if (data) {
    _gpio_led_on(base);
  } else {
    _gpio_led_off(base);
  }
}

static uint32_t _gpio_led_get_data(led_hal_t *base) {
  gpio_led_t *self = (gpio_led_t *)base;
  uint8_t pin_state = GPIO_READ(self->driver);
  return (pin_state == self->active_level) ? 1 : 0;
}
