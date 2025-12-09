/*
 * sys.h
 *
 *  Created on: Dec 9, 2025
 *      Author: 12114
 */

#ifndef SYSTEM_SYS_H_
#define SYSTEM_SYS_H_

#include <stdint.h>

typedef struct {
  // 毫秒级延时
  void (*delay_ms)(uint32_t ms);
  // 微秒级延时
  void (*delay_us)(uint32_t us);
  // 获取系统滴答数（单位：ms）
  uint32_t (*get_systick_ms)(void);
  // 获取系统滴答数（单位：us）
  uint32_t (*get_systick_us)(void);
  // 初始化
  int (*init)(void);
} SysCoreOps;

// 全局接口实例（业务层通过该实例调用接口）
extern const SysCoreOps *g_sys_core_ops;

// 绑定底层实现
void sys_bind_ops(const SysCoreOps *ops);

// 业务层统一调用的封装函数（简化使用）
static inline void sys_delay_ms(uint32_t ms) { g_sys_core_ops->delay_ms(ms); }

static inline void sys_delay_us(uint32_t us) { g_sys_core_ops->delay_us(us); }

static inline uint32_t sys_get_systick_ms(void) {
  return g_sys_core_ops->get_systick_ms();
}

static inline uint32_t sys_get_systick_us(void) {
  return g_sys_core_ops->get_systick_us();
}

static inline int sys_core_init(void) { return g_sys_core_ops->init(); }

#endif /* SYSTEM_SYS_H_ */
