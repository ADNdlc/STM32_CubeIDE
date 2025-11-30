/*
 * led_hal.h
 *
 *  Created on: Nov 29, 2025
 *      Author: 12114
 */

#ifndef HAL_LED_HAL_LED_HAL_H_
#define HAL_LED_HAL_LED_HAL_H_

#include <stdint.h>

// 前向声明
typedef struct led_hal_t led_hal_t;

// 定义虚函数表类型
typedef struct {
  void (*on)(led_hal_t *self);
  void (*off)(led_hal_t *self);
  void (*set_data)(led_hal_t *self, uint32_t data);
  uint32_t (*get_data)(led_hal_t *self);
} led_hal_vtable_t;

// 纯虚类,定义led_hal行为
struct led_hal_t {
  led_hal_vtable_t *vtable;
};

/* ==========================================
 * 内联多态调用辅助函数
 * ========================================== */

/**
 * @brief 打开LED
 * @param self led_hal_t 指针
 */
static inline void led_hal_on(led_hal_t *self) {
  if (self && self->vtable && self->vtable->on) {
    self->vtable->on(self);
  }
}

/**
 * @brief 关闭LED
 * @param self led_hal_t 指针
 */
static inline void led_hal_off(led_hal_t *self) {
  if (self && self->vtable && self->vtable->off) {
    self->vtable->off(self);
  }
}

/**
 * @brief 设置LED数据 (颜色/亮度等)
 * @param self led_hal_t 指针
 * @param data 数据值
 */
static inline void led_hal_set_data(led_hal_t *self, uint32_t data) {
  if (self && self->vtable && self->vtable->set_data) {
    self->vtable->set_data(self, data);
  }
}

/**
 * @brief 获取LED数据
 * @param self led_hal_t 指针
 * @return uint32_t 当前数据值
 */
static inline uint32_t led_hal_get_data(led_hal_t *self) {
  if (self && self->vtable && self->vtable->get_data) {
    return self->vtable->get_data(self);
  }
  return 0;
}

#endif /* HAL_LED_HAL_LED_HAL_H_ */
