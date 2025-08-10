/*
 * speed_test.c
 *
 *  Created on: Jul 27, 2025
 *      Author: 12114
 */

#include "speed_test.h"
#include <stdio.h>
#include <string.h>

// --- DWT (Cycle Counter) 初始化函数 ---
// 需要包含 #include "core_cm7.h" (通常在 stm32h7xx.h 中已包含)
static void DWT_Init(void)
{
    // 使能DWT外设的TRC功能
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    }

    // 复位周期计数器
    DWT->CYCCNT = 0;

    // 使能周期计数器
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

// 定义测试数据块的大小，例如32KB
// 注意：这个值不能超过任何一个内存池的可用大小
#define TEST_DATA_SIZE      (32 * 1024)

/**
 * @brief       测试单个内存区域的读写速度
 * @param       memx: 所属内存块 (来自malloc.h的宏定义)
 * @param       mem_name: 用于打印的内存块名称
 * @retval      无
 */
static void memory_speed_test(uint8_t memx, const char *mem_name)
{
    uint32_t start_cycles, end_cycles, elapsed_cycles;
    float time_ms, speed_mbs;

    printf("\r\n--- Testing Memory: %s ---\r\n", mem_name);

    // 1. 从目标内存区域分配测试缓冲区
    uint8_t *test_buffer = mymalloc(memx, TEST_DATA_SIZE);

    if (test_buffer == NULL)
    {
        printf("Failed to allocate memory from %s. Test skipped.\r\n", mem_name);
        return;
    }
    printf("Succss to allocate memory from %s. Test skipped.\r\n", mem_name);
    // ------------------- 写速度测试 -------------------

    // 在测试前，清理并使数据缓存无效，确保我们测量的是到RAM的真实写入速度
    //SCB_CleanInvalidateDCache();//此工程没有启用Cache,这会导致死机

    // 禁用中断，防止测试被干扰
    __disable_irq();


    start_cycles = DWT->CYCCNT; // 记录开始时的CPU周期数

    // 执行写操作
    for (uint32_t i = 0; i < TEST_DATA_SIZE; i++)
    {
        test_buffer[i] = (uint8_t)i;
    }

    end_cycles = DWT->CYCCNT; // 记录结束时的CPU周期数

    // 恢复中断
    __enable_irq();

    // 计算性能数据
    elapsed_cycles = end_cycles - start_cycles;
    time_ms = (float)elapsed_cycles / (SystemCoreClock / 1000.0f);
    speed_mbs = (float)TEST_DATA_SIZE / 1024.0f / 1024.0f / (time_ms / 1000.0f);
    printf("Write Test: %d bytes | Time: %.3f ms | Speed: %.2f MB/s\r\n", TEST_DATA_SIZE, time_ms, speed_mbs);


    // ------------------- 读速度测试 -------------------

    // 再次清理缓存，确保我们测量的是从RAM的真实读取速度
    //SCB_CleanInvalidateDCache();

    __disable_irq();

    start_cycles = DWT->CYCCNT;

    // 执行读操作
    // volatile 关键字至关重要，防止编译器优化掉整个循环
    volatile uint8_t temp_val;
    for (uint32_t i = 0; i < TEST_DATA_SIZE; i++)
    {
        temp_val = test_buffer[i];
    }

    end_cycles = DWT->CYCCNT;

    __enable_irq();

    (void)temp_val; // 防止编译器警告 "variable set but not used"

    // 计算性能数据
    elapsed_cycles = end_cycles - start_cycles;
    time_ms = (float)elapsed_cycles / (SystemCoreClock / 1000.0f);
    speed_mbs = (float)TEST_DATA_SIZE / 1024.0f / 1024.0f / (time_ms / 1000.0f);
    printf("Read Test:  %d bytes | Time: %.3f ms | Speed: %.2f MB/s\r\n", TEST_DATA_SIZE, time_ms, speed_mbs);

    // 4. 释放内存
    myfree(memx, test_buffer);
}


/**
 * @brief       执行所有内存区域的速度测试
 * @param       无
 * @retval      无
 */
void memory_speed_test_all(void)
{
    printf("\r\n============================================\r\n");
    printf("Starting Memory Performance Benchmark...\r\n");
    printf("System Core Clock: %lu Hz\r\n", SystemCoreClock);
    printf("Test Data Size: %d KB\r\n", TEST_DATA_SIZE / 1024);
    printf("============================================\r\n");

    // 初始化DWT周期计数器11
    DWT_Init();

    // 在-O2或-O3下才能看出DTCM的访问速度,可达200MB/s以上
    // 注意：ITCM是指令紧耦合内存，主要用于存放代码，不做数据读写测试。
    // 对它进行数据操作会通过较慢的总线路径，速度会很慢，这里我们跳过它。
    // 使用uint8_t数据进行测试不能达到32/64位总线的最大吞吐量，只能反应一个相对速度
    // 对外部sdram的写入速度实际上测的是cpu写入FIFO的速度(并不是cpu发起传输到fmc完成操作的真实时间),会异常偏高
    // memory_speed_test(SRAMITCM, "ITCM RAM");

    memory_speed_test(SRAMDTCM, "DTCM RAM");
    memory_speed_test(SRAMIN,   "AXI SRAM (D1)");
    memory_speed_test(SRAM12,   "SRAM1/2 (D2)");
    memory_speed_test(SRAM4,    "SRAM4 (D3)");
    memory_speed_test(SRAMEX,   "External SDRAM");

    printf("\r\n--- Benchmark Finished ---\r\n");
}
