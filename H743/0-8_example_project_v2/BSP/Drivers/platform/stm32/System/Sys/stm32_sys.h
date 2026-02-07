#ifndef PLATFORM_STM32_SYS_STM32_SYS_H_
#define PLATFORM_STM32_SYS_STM32_SYS_H_

#include <stdint.h>

void platform_sys_init(void);
void platform_delay_ms(uint32_t ms);
void platform_delay_us(uint32_t us);
uint32_t platform_get_systick_ms(void);
uint32_t platform_get_systick_us(void);

#endif /* PLATFORM_STM32_SYS_STM32_SYS_H_ */
