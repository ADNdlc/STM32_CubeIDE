/*
 * sys_factory.c
 *
 *  Created on: Dec 9, 2025
 *      Author: Antigravity
 */

#include "sys_factory.h"
#include "factory_config.h"

#if (TARGET_PLATFORM == PLATFORM_STM32)
#include "sys/stm32_sys.h"
#endif

const SysCoreOps *SysFactory_GetOps(void) {
    #if (TARGET_PLATFORM == PLATFORM_STM32)
        return &stm32_sys_core_ops;
    #else
        #error "未定义TARGET_PLATFORM或平台不支持"
    #endif
}