/*
 * Sys.c
 *
 *  Created on: Feb 7, 2026
 *      Author: 12114
 */

#include "sys.h"

const SysOps *g_sys_ops = NULL;

void sys_ops_init(const SysOps *ops) {
	g_sys_ops = ops;
}
