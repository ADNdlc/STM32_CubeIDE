/*
 * gpio_key.c
 *
 *  Created on: May 23, 2025
 *      Author: 12114
 */

#include "gpio_key.h"
#include "sys_hal/sys_hal.h"
#include <stddef.h>
#include <stdlib.h>


// Helper to notify all observers
static void Key_Notify(gpio_key_t *self, KeyEvent event) {
  KeyObserver *current = self->observer_list;
  while (current != NULL) {
    if (current->callback) {
      current->callback(self, event);
    }
    current = current->next;
  }
}

void Key_Init(gpio_key_t *self, gpio_driver_t *port, uint8_t active_level) {
  if (!self || !port)
    return;

  self->port = port;
  self->active_level = active_level;

  // Read initial state
  self->last_state = GPIO_READ(port);
  self->current_state = self->last_state;

  self->last_check_time = 0;
  self->press_start_time = 0;
  self->release_time = 0;
  self->click_count = 0;
  self->long_press_flag = 0;

  // Defaults
  self->debounce_ms = 20;
  self->long_press_ms = 1000;
  self->click_timeout_ms = 300;

  self->observer_list = NULL;
}

gpio_key_t *Key_Create(gpio_driver_t *port, uint8_t active_level) {
  gpio_key_t *key = (gpio_key_t *)malloc(sizeof(gpio_key_t));
  if (key) {
    Key_Init(key, port, active_level);
  }
  return key;
}

void Key_Delete(gpio_key_t *self) {
  if (self) {
    free(self);
  }
}

void Key_SetDebounce(gpio_key_t *self, uint16_t debounce_ms) {
  if (self)
    self->debounce_ms = debounce_ms;
}

void Key_SetLongPress(gpio_key_t *self, uint16_t long_press_ms) {
  if (self)
    self->long_press_ms = long_press_ms;
}

void Key_SetClickTimeout(gpio_key_t *self, uint16_t timeout_ms) {
  if (self)
    self->click_timeout_ms = timeout_ms;
}

void Key_RegisterObserver(gpio_key_t *self, KeyObserver *observer) {
  if (!self || !observer)
    return;

  // Check if already registered to avoid cycles or duplicates
  KeyObserver *current = self->observer_list;
  while (current) {
    if (current == observer)
      return;
    current = current->next;
  }

  // Prepend to list
  observer->next = self->observer_list;
  self->observer_list = observer;
}

void Key_UnregisterObserver(gpio_key_t *self, KeyObserver *observer) {
  if (!self || !observer)
    return;

  KeyObserver **indirect = &self->observer_list;
  while (*indirect != NULL) {
    if (*indirect == observer) {
      *indirect = observer->next;
      observer->next = NULL; // Detach
      return;
    }
    indirect = &(*indirect)->next;
  }
}

void Key_Update(gpio_key_t *self) {
  if (!self || !self->port)
    return;

  uint32_t now = sys_hal_get_tick();
  uint8_t pin_state = GPIO_READ(self->port);

  // Debounce
  if (pin_state != self->last_state) {
    self->last_check_time = now;
    self->last_state = pin_state;
    return;
  }

  if ((now - self->last_check_time) < self->debounce_ms) {
    return;
  }

  // State change confirmed
  if (pin_state != self->current_state) {
    self->current_state = pin_state;

    if (self->current_state == self->active_level) {
      // Pressed
      self->press_start_time = now;
      self->long_press_flag = 0;
    } else {
      // Released
      self->release_time = now;
      if (!self->long_press_flag) {
        self->click_count++;
      }
    }
  }

  // Long press check
  if ((self->current_state == self->active_level) && (!self->long_press_flag) &&
      ((now - self->press_start_time) >= self->long_press_ms)) {

    self->long_press_flag = 1;
    Key_Notify(self, KeyEvent_LongPress);
    self->click_count = 0; // Reset click count
  }

  // Click timeout check
  if ((self->click_count > 0) &&
      ((now - self->release_time) >= self->click_timeout_ms)) {

    switch (self->click_count) {
    case 1:
      Key_Notify(self, KeyEvent_SinglePress);
      break;
    case 2:
      Key_Notify(self, KeyEvent_DoublePress);
      break;
    case 3:
      Key_Notify(self, KeyEvent_TriplePress);
      break;
    default:
      break;
    }
    self->click_count = 0;
  }
}
