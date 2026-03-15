/*
 * gpio_key.c
 *
 *  Created on: May 23, 2025
 *      Author: 12114
 */

#include "gpio_key.h"
#include "Sys.h"
#include <stddef.h>
#include <stdlib.h>
#include "MemPool.h"
#ifdef USE_MEMPOOL
#define GPIOKEY_MEMSOURCE SYS_MEM_INTERNAL // 从哪里分配
#endif

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

  uint32_t now = sys_get_systick_ms();
  self->port = port;
  self->active_level = active_level;
  
  // 根据活跃电平自动设置拉电阻，提高稳定性
  if (active_level == 0) {
    GPIO_SET_MODE(port, GPIO_InputPullUp);
  } else {
    GPIO_SET_MODE(port, GPIO_InputPullDown);
  }

  // 初始硬件状态
  self->last_state = GPIO_READ(port);
  self->current_state = self->last_state;

  self->last_check_time = now;
  self->press_start_time = now;
  self->release_time = now;
  self->click_count = 0;
  self->long_press_flag = 0;

  // 默认设置
  self->debounce_ms = 20;
  self->long_press_ms = 800;
  self->click_timeout_ms = 400;

  self->observer_list = NULL;
}

// 动态创建
gpio_key_t *Key_Create(gpio_driver_t *port, uint8_t active_level) {

#ifdef USE_MEMPOOL
  gpio_key_t *key =
      (gpio_key_t *)sys_malloc(GPIOKEY_MEMSOURCE, sizeof(gpio_key_t));
#else
  gpio_key_t *key = (gpio_key_t *)malloc(sizeof(gpio_key_t));
#endif
  if (key) {
    Key_Init(key, port, active_level);
  }
  return key;
}

// 销毁
void Key_Delete(gpio_key_t *self) {
  if (self) {

#ifdef USE_MEMPOOL
    sys_free(GPIOKEY_MEMSOURCE, self);
#else
    free(self);
#endif
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
    current = current->next; // 遍历到最后一项
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

  KeyObserver **indirect =
      &self->observer_list; // 指向链表项指针 的指针(也是某项的 next成员
                            // 的地址,除第一次遍历)

  while (*indirect != NULL) { // 指向的列表项不为空
    if (*indirect ==
        observer) { // (*indirect)表示某一项的地址值,即上一项的next,的值(除第一次遍历)
      *indirect =
          observer
              ->next; // 将上一项next的值改为待删项的next的值,即让链表跳过待删项
      observer->next = NULL; // 断开,释放待删项next指针(待删项没有释放)
      return;
    }
    indirect = &(*indirect)->next; // 指向下一项的地址值
  }
}

void Key_Update(gpio_key_t *self) {
  if (!self || !self->port)
    return;

  uint32_t now = sys_get_systick_ms();       // 获取当前时间
  uint8_t pin_state = GPIO_READ(self->port); // 读取当前gpio状态

  // 1. 软件消抖状态机
  if (pin_state != self->last_state) {
    self->last_check_time = now;
    self->last_state = pin_state;
  }

  if ((now - self->last_check_time) >= self->debounce_ms) {
    // 确认电平稳定后，再判断逻辑动作
    if (pin_state != self->current_state) {
      self->current_state = pin_state; // 更新稳定后的状态
      
      if (self->current_state == self->active_level) { // 按下
        self->press_start_time = now;
        self->long_press_flag = 0;
      } else { // 释放
        self->release_time = now;
        // 只有非长按后的释放才计入连击
        if (!self->long_press_flag) {
          self->click_count++;
        }
      }
    }
  }

  // 2. 长按实时检查 (按下期间持续判断)
  if ((self->current_state == self->active_level) && (!self->long_press_flag)) {
    if ((now - self->press_start_time) >= self->long_press_ms) {
      self->long_press_flag = 1; 
      self->click_count = 0; // 长按清除连击计数
      Key_Notify(self, KeyEvent_LongPress);
    }
  }

  // 3. 点击/连击触发检查 (必须在按键释放状态下进行)
  if (self->click_count > 0 && self->current_state != self->active_level) {
    if ((now - self->release_time) >= self->click_timeout_ms) {
      if (self->click_count == 1) {
        Key_Notify(self, KeyEvent_SinglePress);
      } else if (self->click_count == 2) {
        Key_Notify(self, KeyEvent_DoublePress);
      } else if (self->click_count == 3) {
        Key_Notify(self, KeyEvent_TriplePress);
      }
      self->click_count = 0;
    }
  }
}
