#include "sys_state.h"
#include <stddef.h>

#define MAX_OBSERVERS 8

static sys_state_t g_state;
static sys_state_observer_cb g_observers[MAX_OBSERVERS];
static int g_observer_count = 0;

static void notify_observers(void) {
  for (int i = 0; i < g_observer_count; i++) {
    if (g_observers[i]) {
      g_observers[i](&g_state);
    }
  }
}

void sys_state_init(void) {
  g_state.volume = 50;
  g_state.brightness = 80;
  g_state.wifi_connected = false;
  g_state.battery_level = 100;
}

const sys_state_t *sys_state_get(void) { return &g_state; }

void sys_state_set_volume(uint8_t vol) {
  if (vol > 100)
    vol = 100;
  if (g_state.volume != vol) {
    g_state.volume = vol;
    notify_observers();
  }
}

void sys_state_set_brightness(uint8_t bri) {
  if (bri > 100)
    bri = 100;
  if (g_state.brightness != bri) {
    g_state.brightness = bri;
    notify_observers();
  }
}

void sys_state_set_wifi(bool connected) {
  if (g_state.wifi_connected != connected) {
    g_state.wifi_connected = connected;
    notify_observers();
  }
}

void sys_state_set_battery(uint8_t level) {
  if (level > 100)
    level = 100;
  if (g_state.battery_level != level) {
    g_state.battery_level = level;
    notify_observers();
  }
}

void sys_state_subscribe(sys_state_observer_cb cb) {
  if (g_observer_count < MAX_OBSERVERS) {
    g_observers[g_observer_count++] = cb;
  }
}
