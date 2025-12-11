#ifndef SYSTEM_SYS_H_
#define SYSTEM_SYS_H_

#include <stddef.h>
#include <stdint.h>

typedef enum {
  SYS_MEM_INTERNAL = 0,
  SYS_MEM_EXTERNAL,
  SYS_MEM_CUSTOM,
} SysMemTag;

typedef struct {
  // 毫秒级延时
  void (*delay_ms)(uint32_t ms);
  // 微秒级延时
  void (*delay_us)(uint32_t us);
  // 获取系统滴答数（单位：ms）
  uint32_t (*get_systick_ms)(void);
  // 获取系统滴答数（单位：us）
  uint32_t (*get_systick_us)(void);
  // 内存分配
  void *(*malloc)(SysMemTag tag, uint32_t size);
  // 内存释放
  void (*free)(SysMemTag tag, void *ptr);
  // 重新分配内存
  void *(*realloc)(SysMemTag tag, void *ptr, uint32_t size);
  // 初始化
  int (*init)(void);
  // 内部内存池初始化
  void (*mem_init_internal)(void);
  // 外部/其他内存池初始化
  void (*mem_init_external)(void);
} SysCoreOps;

// 全局接口实例（业务层通过该实例调用接口）
extern const SysCoreOps *g_sys_core_ops;

// 绑定底层实现
void sys_bind_ops(const SysCoreOps *ops);

// 业务层统一调用的封装函数（简化使用）
static inline void sys_delay_ms(uint32_t ms) { g_sys_core_ops->delay_ms(ms); }

static inline void sys_delay_us(uint32_t us) { g_sys_core_ops->delay_us(us); }

static inline uint32_t sys_get_systick_ms(void) {
  return g_sys_core_ops->get_systick_ms();
}

static inline uint32_t sys_get_systick_us(void) {
  return g_sys_core_ops->get_systick_us();
}

static inline void *sys_malloc(SysMemTag tag, uint32_t size) {
  if (g_sys_core_ops->malloc) {
    return g_sys_core_ops->malloc(tag, size);
  }
  return NULL;
}

static inline void sys_free(SysMemTag tag, void *ptr) {
  if (g_sys_core_ops->free) {
    g_sys_core_ops->free(tag, ptr);
  }
}

static inline void *sys_realloc(SysMemTag tag, void *ptr, uint32_t size) {
  if (g_sys_core_ops->realloc) {
    return g_sys_core_ops->realloc(tag, ptr, size);
  }
  return NULL;
}

static inline int sys_core_init(void) { return g_sys_core_ops->init(); }

static inline void sys_mem_init_internal(void) {
  if (g_sys_core_ops->mem_init_internal) {
    g_sys_core_ops->mem_init_internal();
  }
}

static inline void sys_mem_init_external(void) {
  if (g_sys_core_ops->mem_init_external) {
    g_sys_core_ops->mem_init_external();
  }
}

#endif /* SYSTEM_SYS_H_ */
