/*
 * gpio_key.h
 *
 *  Created on: May 23, 2025
 *      Author: 12114
 */

#ifndef HAL_GPIO_KEY_GPIO_KEY_H_
#define HAL_GPIO_KEY_GPIO_KEY_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "gpio_driver.h"
#include <stdint.h>

//从哪里分配
#define GPIOKEY_MEMSOURCE  SYS_MEM_INTERNAL

typedef struct gpio_key_t gpio_key_t;

typedef enum{
    KeyEvent_SinglePress = 0,
    KeyEvent_DoublePress,
    KeyEvent_TriplePress,
    KeyEvent_LongPress,
  } KeyEvent;

  // 回调原型
  typedef void (*KeyEventCallback)(gpio_key_t *key, KeyEvent event);

  // 观察者(链表)项
  typedef struct KeyObserver
  {
    KeyEventCallback callback;
    struct KeyObserver *next;
  } KeyObserver;

  // 按键类
  struct gpio_key_t
  {
    // 依赖
    gpio_driver_t *port;

    // 配置
    uint8_t active_level; // 0 or 1
    uint16_t debounce_ms;
    uint16_t long_press_ms;
    uint16_t click_timeout_ms;

    // 内部状态
    uint8_t last_state;        // 上一次状态
    uint8_t current_state;     // 当前状态
    uint32_t last_check_time;  // 上一次检查时间
    uint32_t press_start_time; // 按键按下开始时间
    uint32_t release_time;     // 按键释放时间
    uint8_t click_count;       // 点击次数
    uint8_t long_press_flag;   // 长按标志

    // 观察者链表
    KeyObserver *observer_list; // 链表头指针
  };

  // 初始化
  void Key_Init(gpio_key_t *self, gpio_driver_t *port, uint8_t active_level);

  // 动态创建
  gpio_key_t *Key_Create(gpio_driver_t *port, uint8_t active_level);
  void Key_Delete(gpio_key_t *self);

  // 参数设置
  void Key_SetDebounce(gpio_key_t *self, uint16_t debounce_ms);
  void Key_SetLongPress(gpio_key_t *self, uint16_t long_press_ms);
  void Key_SetClickTimeout(gpio_key_t *self, uint16_t timeout_ms);

  // 注册观察者
  void Key_RegisterObserver(gpio_key_t *self, KeyObserver *observer);
  void Key_UnregisterObserver(gpio_key_t *self, KeyObserver *observer);

  // Main Loop Update
  void Key_Update(gpio_key_t *self);

#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_KEY_GPIO_KEY_H_ */
