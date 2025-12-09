/*
 * sys.c
 *
 *  Created on: Dec 9, 2025
 *      Author: 12114
 */

#include "sys.h"
#include <stddef.h>

const SysCoreOps *g_sys_core_ops = NULL;

void sys_bind_ops(const SysCoreOps *ops) {
	g_sys_core_ops = ops;
}
