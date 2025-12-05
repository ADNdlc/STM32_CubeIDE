/*
 * gpio_key.h
 *
 *  Created on: May 23, 2025
 *      Author: 12114
 */

#ifndef HAL_GPIO_KEY_GPIO_KEY_H_
#define HAL_GPIO_KEY_GPIO_KEY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "gpio_driver.h"
#include <stdint.h>


typedef struct gpio_key_t gpio_key_t;

typedef enum {
  KeyEvent_SinglePress = 0,
  KeyEvent_DoublePress,
  KeyEvent_TriplePress,
  KeyEvent_LongPress,
} KeyEvent;

// Callback type
typedef void (*KeyEventCallback)(gpio_key_t *key, KeyEvent event);

// Observer node
// Caller is responsible for allocating this node (can be static)
typedef struct KeyObserver {
  KeyEventCallback callback;
  struct KeyObserver *next;
} KeyObserver;

struct gpio_key_t {
  // Dependencies
  gpio_driver_t *port;

  // Configuration
  uint8_t active_level; // 0 or 1
  uint16_t debounce_ms;
  uint16_t long_press_ms;
  uint16_t click_timeout_ms;

  // State
  uint8_t last_state;
  uint8_t current_state;
  uint32_t last_check_time;
  uint32_t press_start_time;
  uint32_t release_time;
  uint8_t click_count;
  uint8_t long_press_flag;

  // Observers
  KeyObserver *observer_list;
};

// Initialization
// Initialize a statically allocated key structure
void Key_Init(gpio_key_t *self, gpio_driver_t *port, uint8_t active_level);

// Dynamic allocation (optional, wraps Key_Init)
gpio_key_t *Key_Create(gpio_driver_t *port, uint8_t active_level);
void Key_Delete(gpio_key_t *self);

// Configuration
void Key_SetDebounce(gpio_key_t *self, uint16_t debounce_ms);
void Key_SetLongPress(gpio_key_t *self, uint16_t long_press_ms);
void Key_SetClickTimeout(gpio_key_t *self, uint16_t timeout_ms);

// Observer Management
// observer node must be kept alive by the caller as long as it is registered
void Key_RegisterObserver(gpio_key_t *self, KeyObserver *observer);
void Key_UnregisterObserver(gpio_key_t *self, KeyObserver *observer);

// Main Loop Update
void Key_Update(gpio_key_t *self);

#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_KEY_GPIO_KEY_H_ */
