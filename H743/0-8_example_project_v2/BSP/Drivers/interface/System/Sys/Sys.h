/*
 * Sys.h
 *
 *  Created on: Feb 7, 2026
 *      Author: 12114
 */

#ifndef DRIVERS_INTERFACE_SYSTEM_SYS_SYS_H_
#define DRIVERS_INTERFACE_SYSTEM_SYS_SYS_H_

#include <stddef.h>
#include <stdint.h>

typedef struct {
  void (*sys_init)(void);
  // 毫秒级延时
  void (*sys_delay_ms)(uint32_t ms);
  // 微秒级延时
  void (*sys_delay_us)(uint32_t us);
  // 获取系统滴答数（单位：ms）
  uint32_t (*sys_get_systick_ms)(void);
  // 获取系统滴答数（单位：us）
  uint32_t (*sys_get_systick_us)(void);
} SysOps;

// 全局接口实例（业务层通过该实例调用接口）
extern const SysOps *g_sys_ops;
// 系统层功能初始化
void sys_ops_init(const SysOps *ops);


/* ----- 业务层统一调用的封装函数 ----- */
static inline void sys_init(void){g_sys_ops->sys_init();}

static inline void sys_delay_ms(uint32_t ms) { g_sys_ops->sys_delay_ms(ms); }

static inline void sys_delay_us(uint32_t us) { g_sys_ops->sys_delay_us(us); }

static inline uint32_t sys_get_systick_ms(void) {
  return g_sys_ops->sys_get_systick_ms();
}

static inline uint32_t sys_get_systick_us(void) {
  return g_sys_ops->sys_get_systick_us();
}


#endif /* DRIVERS_INTERFACE_SYSTEM_SYS_SYS_H_ */
