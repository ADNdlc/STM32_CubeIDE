#ifndef _SYS_STATE_H
#define _SYS_STATE_H

#include <stdbool.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

// --- System Properties ---

typedef struct {
  uint8_t volume;     // 0-100
  uint8_t brightness; // 0-100
  bool wifi_connected;
  bool bluetooth_connected;
  uint8_t battery_level;
} sys_state_t;

// Init
void sys_state_init(void);

// Getters
const sys_state_t *sys_state_get(void);

// Setters (will trigger events)
void sys_state_set_volume(uint8_t vol);
void sys_state_set_brightness(uint8_t bri);
void sys_state_set_wifi(bool connected);
void sys_state_set_battery(uint8_t level);

// Observer Pattern for UI updates
typedef void (*sys_state_observer_cb)(const sys_state_t *state);
void sys_state_subscribe(sys_state_observer_cb cb);

#ifdef __cplusplus
}
#endif

#endif
