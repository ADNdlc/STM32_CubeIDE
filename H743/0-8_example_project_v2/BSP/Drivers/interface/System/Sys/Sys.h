#ifndef DRIVERS_INTERFACE_SYSTEM_SYS_SYS_H_
#define DRIVERS_INTERFACE_SYSTEM_SYS_SYS_H_

#include <stddef.h>
#include <stdint.h>

#include "System_port.h" // 创建不同头文件来选择不同platform_xxx函数

/*
void platform_sys_init(void);
void platform_delay_ms(uint32_t ms);
void platform_delay_us(uint32_t us);
uint32_t platform_get_systick_ms(void);
uint32_t platform_get_systick_us(void);
uint32_t platform_get_CoreClock(void);
*/

/*
 * 系统层功能统一接口
 * 采用宏定义，直接映射到 platform_xxx 实现。
 */
#define sys_init() platform_sys_init()
#define sys_delay_ms(ms) platform_delay_ms(ms)
#define sys_delay_us(us) platform_delay_us(us)
#define sys_get_systick_ms() platform_get_systick_ms()
#define sys_get_systick_us() platform_get_systick_us()
#define sys_get_CoreClock() platform_get_CoreClock() // 获取系统时钟频率(MHz)

#endif /* DRIVERS_INTERFACE_SYSTEM_SYS_SYS_H_ */
