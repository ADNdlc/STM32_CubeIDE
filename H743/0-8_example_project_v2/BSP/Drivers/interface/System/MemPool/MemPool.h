/*
 * mempool.h
 *
 *  Created on: Feb 7, 2026
 *      Author: 12114
 */

#ifndef DRIVERS_INTERFACE_SYSTEM_MEMPOOL_MEMPOOL_H_
#define DRIVERS_INTERFACE_SYSTEM_MEMPOOL_MEMPOOL_H_

#include <stddef.h>
#include <stdint.h>

/* 内存池号 */
typedef enum {
  SYS_MEM_INTERNAL = 0,
  SYS_MEM_EXTERNAL,
  SYS_MEM_CUSTOM,
} SysMemTag;

typedef struct {
  // 内部内存池初始化
  void (*mem_init_internal)(void);
  // 外部/其他内存池初始化
  void (*mem_init_external)(void); // 外部内存池的初始化要在对应RAM设备初始化完成后
  // 内存分配
  void *(*malloc)(SysMemTag tag, uint32_t size);
  // 内存释放
  void (*free)(SysMemTag tag, void *ptr);
  // 重新分配内存
  void *(*realloc)(SysMemTag tag, void *ptr, uint32_t size);
} SysMem;

// 全局接口实例（业务层通过该实例调用接口）
extern const SysMem *g_sys_mem;
// 内存池功能初始化
void sys_mem_init(const SysMem *mem);


/* ----- 业务层统一调用的封装函数 ----- */
static inline void sys_mem_init_internal(void) {
  if (g_sys_mem->mem_init_internal) {
	  g_sys_mem->mem_init_internal();
  }
}

static inline void sys_mem_init_external(void) {
  if (g_sys_mem->mem_init_external) {
	  g_sys_mem->mem_init_external();
  }
}

static inline void *sys_malloc(SysMemTag tag, uint32_t size) {
  if (g_sys_mem->malloc) {
    return g_sys_mem->malloc(tag, size);
  }
  return NULL;
}

static inline void sys_free(SysMemTag tag, void *ptr) {
  if (g_sys_mem->free) {
	  g_sys_mem->free(tag, ptr);
  }
}

static inline void *sys_realloc(SysMemTag tag, void *ptr, uint32_t size) {
  if (g_sys_mem->realloc) {
    return g_sys_mem->realloc(tag, ptr, size);
  }
  return NULL;
}

#endif /* DRIVERS_INTERFACE_SYSTEM_MEMPOOL_MEMPOOL_H_ */
