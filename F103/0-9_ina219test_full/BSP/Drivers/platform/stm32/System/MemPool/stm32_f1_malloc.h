#ifndef __MALLOC_H
#define __MALLOC_H

#include <stdint.h>

#ifndef NULL
#define NULL 0
#endif

//#define USE_EXTERNAL_MEM

/* 定义两个内存池 */
#define SRAMIN      0       /* 内部SRAM, 共64KB */
#ifdef USE_EXTERNAL_MEM
#define SRAMEX      1       /* SRAMEX内存池, 外扩SRAM, 共1024KB */

#define SRAMBANK    2       /* 定义支持的SRAM块数. */
#else
#define SRAMBANK    1       /* 定义支持的SRAM块数. */
#endif

/* 定义内存管理表类型,当外扩SDRAM的时候，必须使用uint32_t类型，否则可以定义成uint16_t，以节省内存占用 */
#ifdef USE_EXTERNAL_MEM
#define MT_TYPE     uint32_t
#else
#define MT_TYPE     uint16_t
#endif
/* 单块内存，内存管理所占用的全部空间大小计算公式如下：
 * size = MEM1_MAX_SIZE + (MEM1_MAX_SIZE / MEM1_BLOCK_SIZE) * sizeof(MT_TYPE)
 * 以SRAMEX为例，size = 963 * 1024 + (963 * 1024 / 32) * 2 = 1047744 ≈ 1023KB

 * 已知总内存容量(size)，最大内存池的计算公式如下：
 * MEM1_MAX_SIZE = (MEM1_BLOCK_SIZE * size) / (MEM1_BLOCK_SIZE + sizeof(MT_TYPE))
 * 以SRAMIN为例,MEM1_MAX_SIZE = (32 * 64) / (32 + 2) = 60.24KB ≈ 60KB
 * 但是我们为了给其他全局变量 / 数组等预留内存空间, 这里设置最大管理为 40KB
 */

/* mem1内存参数设定.mem1是F103内部的SRAM. */
#define MEM1_BLOCK_SIZE         32                              /* 内存块大小为32字节 */
#define MEM1_MAX_SIZE           5 * 1024                       /* 最大管理内存 40K, F103内部SRAM总共64KB */
#define MEM1_ALLOC_TABLE_SIZE   MEM1_MAX_SIZE/MEM1_BLOCK_SIZE   /* 内存表大小 */
#ifdef USE_EXTERNAL_MEM
/* mem2内存参数设定.mem3是F103外扩SRAM */
#define MEM2_BLOCK_SIZE         32                              /* 内存块大小为32字节 */
#define MEM2_MAX_SIZE           800 *1024                       /* 最大管理内存963K, F103外扩SRAM大小1024KB */
#define MEM2_ALLOC_TABLE_SIZE   MEM2_MAX_SIZE/MEM2_BLOCK_SIZE   /* 内存表大小 */
#endif
/* 内存管理控制器 */
struct _m_mallco_dev
{
    void (*init)(uint8_t);          /* 初始化 */
    uint16_t (*perused)(uint8_t);   /* 内存使用率 */
    uint8_t *membase[SRAMBANK];     /* 内存池 管理SRAMBANK个区域的内存 */
    MT_TYPE *memmap[SRAMBANK];      /* 内存管理状态表 */
    uint8_t  memrdy[SRAMBANK];      /* 内存管理是否就绪 */
};

extern struct _m_mallco_dev mallco_dev; /* 在mallco.c里面定义 */


/* 用户调用函数 */
void my_mem_init(uint8_t memx);                     /* 内存管理初始化函数(外/内部调用) */
uint16_t my_mem_perused(uint8_t memx) ;             /* 获得内存使用率(外/内部调用) */
void my_mem_set(void *s, uint8_t c, uint32_t count);/* 内存设置函数 */
void my_mem_copy(void *des, void *src, uint32_t n); /* 内存拷贝函数 */

void myfree(uint8_t memx, void *ptr);               /* 内存释放(外部调用) */
void *mymalloc(uint8_t memx, uint32_t size);        /* 内存分配(外部调用) */
void *myrealloc(uint8_t memx, void *ptr, uint32_t size);    /* 重新分配内存(外部调用) */

#endif
