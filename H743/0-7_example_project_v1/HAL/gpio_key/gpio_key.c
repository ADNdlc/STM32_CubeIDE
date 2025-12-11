/*
 * gpio_key.c
 *
 *  Created on: May 23, 2025
 *      Author: 12114
 */

#include "gpio_key.h"
#include "sys.h"
#include <stddef.h>
#include <stdlib.h>


// 调用对象的所有观察者
static void Key_Notify(gpio_key_t *self, KeyEvent event) {
  KeyObserver *current = self->observer_list;
  // 遍历链表
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

  // 初始硬件状态
  self->last_state = GPIO_READ(port);
  self->current_state = self->last_state;

  self->last_check_time = 0;
  self->press_start_time = 0;
  self->release_time = 0;
  self->click_count = 0;
  self->long_press_flag = 0;

  // 默认设置
  self->debounce_ms = 20;
  self->long_press_ms = 1000;
  self->click_timeout_ms = 300;

  self->observer_list = NULL;
}

// 动态创建
gpio_key_t *Key_Create(gpio_driver_t *port, uint8_t active_level) {
  gpio_key_t *key = (gpio_key_t *)sys_malloc(GPIOKEY_MEMSOURCE, sizeof(gpio_key_t));
  if (key) {
    Key_Init(key, port, active_level);
  }
  return key;
}

// 销毁
void Key_Delete(gpio_key_t *self) {
  if (self) {
    sys_free(GPIOKEY_MEMSOURCE, self);
  }
}

/* 设置消抖时间
 */
void Key_SetDebounce(gpio_key_t *self, uint16_t debounce_ms) {
  if (self)
    self->debounce_ms = debounce_ms;
}

/* 设置长按触发时间
 */
void Key_SetLongPress(gpio_key_t *self, uint16_t long_press_ms) {
  if (self)
    self->long_press_ms = long_press_ms;
}

/* 设置点击超时时间(触发连击的点击间隔要短于此时间)
 */
void Key_SetClickTimeout(gpio_key_t *self, uint16_t timeout_ms) {
  if (self)
    self->click_timeout_ms = timeout_ms;
}

/* 注册观察者
 */
void Key_RegisterObserver(gpio_key_t *self, KeyObserver *observer) {
  if (!self || !observer)
    return;

  // 避免重复注册导致重复调用
  KeyObserver *current = self->observer_list;
  while (current) {
    if (current == observer)
      return;
    current = current->next; //遍历到最后一项
  }

  // 添加到链表尾
  observer->next = self->observer_list;
  self->observer_list = observer;
}

/* 注销观察者(按地址匹配)
 */
void Key_UnregisterObserver(gpio_key_t *self, KeyObserver *observer) { 
  if (!self || !observer)
    return;

  KeyObserver **indirect = &self->observer_list; //指向链表项指针 的指针(也是某项的 next成员 的地址,除第一次遍历)
  
  while (*indirect != NULL) {    // 指向的列表项不为空
    if (*indirect == observer) { // (*indirect)表示某一项的地址值,即上一项的next,的值(除第一次遍历)
      *indirect = observer->next;// 将上一项next的值改为待删项的next的值,即让链表跳过待删项
      observer->next = NULL;     // 断开,释放待删项next指针(待删项没有释放)
      return;
    }
    indirect = &(*indirect)->next; //指向下一项的地址值
  }
}

void Key_Update(gpio_key_t *self) {
  if (!self || !self->port)
    return;

  uint32_t now = sys_get_systick_ms(); // 获取当前时间
  uint8_t pin_state = GPIO_READ(self->port);// 读取当前gpio状态

  
  if (pin_state != self->last_state) { //发生动作后立刻返回
    self->last_check_time = now;
    self->last_state = pin_state;
    return;
  }
  if ((now - self->last_check_time) < self->debounce_ms) { //等待稳定
    return;
  }


  // 确认产生动作
  if (pin_state != self->current_state) {
    self->current_state = pin_state; // 更新状态
    // 判断动作类型
    if (self->current_state == self->active_level) { // 按下
      // 记录此次按下时间
      self->press_start_time = now;
      self->long_press_flag = 0;
    } else {                          // 释放                     
      // 记录此次释放时间
      self->release_time = now;
      if (!self->long_press_flag) {
        self->click_count++;
      }
    }
  }

  // 长按检查(如果当前是按下状态,没有长按标注,按下时间超过长按触发时间)
  if ((self->current_state == self->active_level) && (!self->long_press_flag) &&
      ((now - self->press_start_time) >= self->long_press_ms)) {

    self->long_press_flag = 1; //长按置位,防止反复调用
    Key_Notify(self, KeyEvent_LongPress);
    self->click_count = 0;
  }

  // 判断点击次数
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
