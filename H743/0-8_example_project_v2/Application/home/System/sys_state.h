#ifndef _SYS_STATE_H
#define _SYS_STATE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief System State
 *
 * 此模块负责维护系统运行时的全局状态，如音量、亮度等。并提供状态改变时的观察者通知机制(UI更新)。
 *
 */

// --- 系统属性 ---
typedef struct {
  uint8_t volume;     // 0-100
  uint8_t brightness; // 0-100
  bool wifi_connected;
  bool mqtt_connected;
  bool bluetooth_connected;
  uint8_t battery_level; // 0-100
  //...
} sys_state_t;

// Init
void sys_state_init(void);

// 获取状态
const sys_state_t *sys_state_get(void);

// 设置状态属性
void sys_state_set_volume(uint8_t vol);
void sys_state_set_brightness(uint8_t bri);
void sys_state_set_wifi(bool connected);
void sys_state_set_mqtt(bool connected);
void sys_state_set_battery(uint8_t level);

// 观察者模式
typedef void (*sys_state_observer_cb)(
    const sys_state_t *state);                      // 观察者回调原型
void sys_state_subscribe(sys_state_observer_cb cb); // 订阅状态更新

#ifdef __cplusplus
}
#endif

#endif
