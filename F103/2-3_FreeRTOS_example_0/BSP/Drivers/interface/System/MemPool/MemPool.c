/*
 * mempool.c
 *
 *  Created on: Feb 7, 2026
 *      Author: 12114
 */

#include "MemPool.h"

const SysMem *g_sys_mem = NULL;

void sys_mem_init(const SysMem *mem) {
	g_sys_mem = mem;
}
