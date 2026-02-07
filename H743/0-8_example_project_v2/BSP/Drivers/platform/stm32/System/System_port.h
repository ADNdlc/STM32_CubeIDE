#ifndef PLATFORM_STM32_SYS_PORT_H_
#define PLATFORM_STM32_SYS_PORT_H_

#include "Project_cfg.h"
#if (TARGET_PLATFORM == PLATFORM_STM32)
/*
 * STM32 平台桥接
 * 这里通过包含具体的 stm32_sys.h 来提供 platform_xxx 实现。
 */
#include "stm32_sys.h"
#else
#error "System_port: 未定义有效的目标平台，请检查TARGET_PLATFORM配置"
#endif

#endif /* PLATFORM_STM32_SYS_PORT_H_ */
