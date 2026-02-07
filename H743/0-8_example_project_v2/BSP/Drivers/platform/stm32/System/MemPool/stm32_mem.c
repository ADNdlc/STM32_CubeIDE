/*
 * stm32_mem.c
 *
 *  Created on: Feb 7, 2026
 *      Author: 12114
 */

#include "stm32_mem.h"
#include "stm32_h7_malloc.h"

// 平台内部配置
#define STM32_MEM_INTERNAL	SRAMIN
#define STM32_MEM_EXTERNAL	SRAMEX
#define STM32_MEM_CUSTOM	SRAMDTCM //DTCM仅CPU和MDMA(通过AHBS)可以访问

static void *stm32_mem_malloc(SysMemTag tag, uint32_t size) {
  uint8_t memx;
  switch (tag) {
  case SYS_MEM_INTERNAL:
    memx = STM32_MEM_INTERNAL;
    break;
  case SYS_MEM_EXTERNAL:
    memx = STM32_MEM_EXTERNAL;
    break;
  case SYS_MEM_CUSTOM:
    memx = STM32_MEM_CUSTOM;
    break;
  default:
    return NULL;
  }
  return mymalloc(memx, size);
}

static void stm32_mem_free(SysMemTag tag, void *ptr) {
  uint8_t memx;
  switch (tag) {
  case SYS_MEM_INTERNAL:
    memx = STM32_MEM_INTERNAL;
    break;
  case SYS_MEM_EXTERNAL:
    memx = STM32_MEM_EXTERNAL;
    break;
  case SYS_MEM_CUSTOM:
    memx = STM32_MEM_CUSTOM;
    break;
  default:
    return;
  }
  myfree(memx, ptr);
}

static void *stm32_mem_realloc(SysMemTag tag, void *ptr, uint32_t size) {
  uint8_t memx;
  switch (tag) {
  case SYS_MEM_INTERNAL:
    memx = STM32_MEM_INTERNAL;
    break;
  case SYS_MEM_EXTERNAL:
    memx = STM32_MEM_EXTERNAL;
    break;
  case SYS_MEM_CUSTOM:
    memx = STM32_MEM_CUSTOM;
    break;
  default:
    return NULL;
  }
  return myrealloc(memx, ptr, size);
}

static void stm32_sys_mem_init_internal(void) {
  my_mem_init(STM32_MEM_INTERNAL);
  my_mem_init(STM32_MEM_CUSTOM);
}

static void stm32_sys_mem_init_external(void) {
  my_mem_init(STM32_MEM_EXTERNAL);
}

const SysMem stm32_sys_mem = {
	.mem_init_internal = stm32_sys_mem_init_internal,
	.mem_init_internal = stm32_sys_mem_init_external,
	.malloc = stm32_mem_malloc,
	.free = stm32_mem_free,
	.realloc = stm32_mem_realloc
};

SysMem *stm32_sys_mem_create(void){
	return (SysMem *)&stm32_sys_mem;
}

