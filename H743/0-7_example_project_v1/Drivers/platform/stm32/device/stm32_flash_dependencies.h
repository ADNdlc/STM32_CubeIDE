/*
 * stm32_flash_dependencies.h
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  STM32 平台 Flash 驱动依赖注入实现
 */

#ifndef PLATFORM_STM32_DEVICE_STM32_FLASH_DEPENDENCIES_H_
#define PLATFORM_STM32_DEVICE_STM32_FLASH_DEPENDENCIES_H_

#include "flash_dependencies.h"

#ifdef __cplusplus
extern "C" {
#endif

// 创建 STM32 平台默认依赖
// 使用 sys_delay_ms 和 elog
flash_dependencies_t *stm32_flash_create_dependencies(void);

// 销毁 STM32 平台依赖
void stm32_flash_destroy_dependencies(flash_dependencies_t *deps);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_STM32_DEVICE_STM32_FLASH_DEPENDENCIES_H_ */
