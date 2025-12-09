/*
 * sys_factory.c
 *
 *  Created on: Dec 9, 2025
 *      Author: Antigravity
 */

#include "sys_factory.h"
#include "sys/stm32_sys.h"

const SysCoreOps *SysFactory_GetOps(void) {
	return &stm32_sys_core_ops;
}
