#include "sys_state.h"
#include <stddef.h>
#include "elog.h"

#define LOG_TAG "SYS_STATE"
#include "elog.h"

#define MAX_OBSERVERS 8

static sys_state_t g_state;   // 系统状态实例
static sys_state_observer_cb g_observers[MAX_OBSERVERS];  // 观察者列表
static int g_observer_count = 0;                          // 观察者数量

// 调用所有观察者回调
static void notify_observers(void) {
  for (int i = 0; i < g_observer_count; i++) {
    if (g_observers[i]) {
      g_observers[i](&g_state);
    }
  }
  log_d("Notified %d observers", g_observer_count);
}

/**
 * @brief 系统状态初始化
 * 
 */ 
void sys_state_init(void) {
  g_state.volume = 50;
  g_state.brightness = 80;
  g_state.wifi_connected = false;
  g_state.battery_level = 100;
  log_i("System state initialized");
}

/**
 * @brief 获取系统状态
 * 
 * @return const sys_state_t* 
 */
const sys_state_t *sys_state_get(void) { 
  log_d("State accessed");
  return &g_state; 
}

/**
 * @brief 设置系统音量
 * 
 * @param vol 0-100
 */
void sys_state_set_volume(uint8_t vol) {
  if (vol > 100)
    vol = 100;
  if (g_state.volume != vol) {
    g_state.volume = vol;
    notify_observers();
  }
}

/**
 * @brief 设置系统亮度
 * 
 * @param bri 0-100
 */
void sys_state_set_brightness(uint8_t bri) {
  if (bri > 100)
    bri = 100;
  if (g_state.brightness != bri) {
    g_state.brightness = bri;
    notify_observers();
  }
}

/**
 * @brief 设置WiFi连接状态
 * 
 * @param connected 
 */
void sys_state_set_wifi(bool connected) {
  if (g_state.wifi_connected != connected) {
    g_state.wifi_connected = connected;
    notify_observers();
  }
}

/**
 * @brief 设置电量
 * 
 * @param level 0-100
 */
void sys_state_set_battery(uint8_t level) {
  if (level > 100)
    level = 100;
  if (g_state.battery_level != level) {
    g_state.battery_level = level;
    notify_observers();
  }
}

/**
 * @brief 订阅系统状态更新
 * 
 * @param cb 观察者回调函数
 */
void sys_state_subscribe(sys_state_observer_cb cb) {
  if (g_observer_count < MAX_OBSERVERS) {
    g_observers[g_observer_count++] = cb;
  }
}
