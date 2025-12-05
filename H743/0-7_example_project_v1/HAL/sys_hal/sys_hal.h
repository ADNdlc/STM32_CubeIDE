/*
 * sys_hal.h
 *
 *  Created on: Dec 5, 2025
 *      Author: 12114
 */

#ifndef HAL_SYS_HAL_SYS_HAL_H_
#define HAL_SYS_HAL_SYS_HAL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 系统驱动接口
typedef struct {
  uint32_t (*get_tick)(void);
  void (*delay)(uint32_t ms);
} sys_driver_t;

// 初始化系统HAL
void sys_hal_init(const sys_driver_t *driver);

// 获取系统Tick (ms)
uint32_t sys_hal_get_tick(void);

// 延时 (ms)
void sys_hal_delay(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* HAL_SYS_HAL_SYS_HAL_H_ */
