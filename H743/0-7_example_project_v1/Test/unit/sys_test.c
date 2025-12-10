#include "all_tests_config.h"

#if _sys_test_
#include "stm32h7xx_hal.h"
#include "elog.h"
#include "sys.h"
#include "sys_factory.h"
#include "gpio_factory.h"
#include "gpio_led/gpio_led.h"
#include "led_hal.h"

/* -- 辅助函数(依赖stm32core) -- */
// --- DWT (Cycle Counter) 初始化函数 ---
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
#define TEST_DATA_SIZE (32 * 1024)

/**
 * @brief       测试单个内存区域的读写速度
 * @param       memx: 所属内存块 (来自sys宏定义)
 * @param       mem_name: 用于打印的内存块名称
 * @retval      无
 */
static void memory_speed_test(uint8_t memx, const char *mem_name)
{
    uint32_t start_cycles, end_cycles, elapsed_cycles;
    uint32_t time_us, time_ms;
    uint32_t speed_kbps;

    log_i("\r\n--- Testing Memory: %s ---\r\n", mem_name);
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(5);

    // 1. 从目标内存区域分配测试缓冲区
    uint8_t *test_buffer = sys_malloc(memx, TEST_DATA_SIZE);

    if (test_buffer == NULL)
    {
        log_i("Failed to allocate memory from %s. Test skipped.\r\n", mem_name);
        return;
    }
    log_i("Success to allocate memory from %s.\r\n", mem_name);
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(5);
    
    // 更新SystemCoreClock值以确保准确性
    SystemCoreClockUpdate();
    log_i("System Core Clock: %lu Hz\r\n", SystemCoreClock);
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(5);
    
    // ------------------- 写速度测试 -------------------

    // 在测试前，清理并使数据缓存无效，确保我们测量的是到RAM的真实写入速度
    // SCB_CleanInvalidateDCache();//此工程没有启用Cache,这会导致死机

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

    // 计算性能数据（避免浮点运算）
    elapsed_cycles = end_cycles - start_cycles;
    
    // 时间计算: cycles / (cycles/ms) = ms
    // 或者: (cycles * 1000000) / (cycles/s) = us
    if (SystemCoreClock > 0) {
        time_us = (uint32_t)(((uint64_t)elapsed_cycles * 1000000) / SystemCoreClock);
        time_ms = time_us / 1000;
        // 速度计算: (bytes) / (seconds) = bytes/s => KB/s
        // speed_kbps = (TEST_DATA_SIZE * 1000) / time_ms; 
        speed_kbps = (time_ms > 0) ? (TEST_DATA_SIZE / time_ms) : 0;
        log_i("Write Test: %d bytes | Time: %lu ms (%lu us) | Speed: %lu KB/s\r\n", 
              TEST_DATA_SIZE, time_ms, time_us, speed_kbps);
    } else {
        log_i("Write Test: %d bytes | Time: N/A | Speed: N/A (SystemCoreClock=0)\r\n", TEST_DATA_SIZE);
    }
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(10);

    // ------------------- 读速度测试 -------------------

    // 再次清理缓存，确保我们测量的是从RAM的真实读取速度
    // SCB_CleanInvalidateDCache();

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

    // 计算性能数据（避免浮点运算）
    elapsed_cycles = end_cycles - start_cycles;
    
    if (SystemCoreClock > 0) {
        time_us = (uint32_t)(((uint64_t)elapsed_cycles * 1000000) / SystemCoreClock);
        time_ms = time_us / 1000;
        // speed_kbps = (TEST_DATA_SIZE * 1000) / time_ms;
        speed_kbps = (time_ms > 0) ? (TEST_DATA_SIZE / time_ms) : 0;
        log_i("Read Test:  %d bytes | Time: %lu ms (%lu us) | Speed: %lu KB/s\r\n", 
              TEST_DATA_SIZE, time_ms, time_us, speed_kbps);
    } else {
        log_i("Read Test:  %d bytes | Time: N/A | Speed: N/A (SystemCoreClock=0)\r\n", TEST_DATA_SIZE);
    }
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(10);

    // 4. 释放内存
    sys_free(memx, test_buffer);
}

/* -- 测试函数(依赖stm32core) -- */
void RamSpeed_test(void)
{
    log_i("Starting Memory Performance Benchmark...\r\n");
    log_i("Test Data Size: %d KB\r\n", TEST_DATA_SIZE / 1024);
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(10);

    // 初始化DWT周期计数器
    DWT_Init();

    // 使用uint8_t数据进行测试不能达到32/64位总线的最大吞吐量，只能反应一个相对速度
    // 对外部sdram的写入速度实际上测的是cpu写入FIFO的速度(并不是cpu发起传输到fmc完成传输操作的真实时间),会异常偏高
    memory_speed_test(SYS_MEM_INTERNAL, "SYS_MEM_INTERNAL");
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(20);
    
    memory_speed_test(SYS_MEM_EXTERNAL, "SYS_MEM_EXTERNAL");
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(20);
    
    memory_speed_test(SYS_MEM_CUSTOM, "SYS_MEM_CUSTOM");

    log_i("\r\n--- Benchmark Finished ---\r\n");
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(10);
}

void base_test(void) // delay,get_systick,...
{
    gpio_driver_t *driver = gpio_driver_get(GPIO_LED_0);
    if (driver)
    {
        gpio_led_t *led = gpio_led_create(driver, 0);
        for (int i = 0; i < 3; i++)
        {
            led_hal_on((led_hal_t *)led);
            sys_delay_ms(300);
            led_hal_off((led_hal_t *)led);
            sys_delay_ms(300);
        }
        gpio_led_delete(led);
        sys_delay_ms(500);
    }

    // 测试 sys_delay_ms 函数
    int systick = sys_get_systick_ms();
    log_i("systick: %d\r\n", systick);
    sys_delay_ms(1000);
    systick = sys_get_systick_ms();
    log_i("systick: %d\r\n", systick);
    sys_delay_ms(500);

    // 测试 sys_delay_us 函数
    log_i("Testing sys_delay_us function...\r\n");
    uint32_t start_us = sys_get_systick_us(); // 输出数据查看效果更明显
    sys_delay_us(1000);                       // 延时1000微秒(1毫秒)
    uint32_t end_us = sys_get_systick_us();
    log_i("Delay 1000 us, actually took %u us\r\n", end_us - start_us);
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(10);
}

void BaseRead_test(void)
{
//    log_i("\r\n============================================\r\n");
    log_i("Testing Base System Functions...\r\n");
//    log_i("============================================\r\n");
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(10);

    // 测试 malloc/free/realloc 函数基本读取写入(SYS_MEM_INTERNAL)
    log_i("Testing sys_malloc, sys_free and sys_realloc functions...\r\n");

    // 分配内存
    uint8_t *test_mem = (uint8_t *)sys_malloc(SYS_MEM_INTERNAL, 128);
    if (test_mem != NULL)
    {
        log_i("Successfully allocated 128 bytes from SYS_MEM_INTERNAL\r\n");

        // 初始化内存内容
        for (int i = 0; i < 128; i++)
        {
            test_mem[i] = i;
        }

        // 测试 realloc 扩大内存
        uint8_t *larger_mem = (uint8_t *)sys_realloc(SYS_MEM_INTERNAL, test_mem, 256);
        if (larger_mem != NULL)
        {
            log_i("Successfully reallocated to 256 bytes\r\n");

            // 验证原来的数据是否还在
            int data_valid = 1;
            for (int i = 0; i < 128; i++)
            {
                if (larger_mem[i] != i)
                {
                    data_valid = 0;
                    break;
                }
            }

            if (data_valid)
            {
                log_i("Original data preserved after reallocation\r\n");
            }
            else
            {
                log_i("Data corruption detected after reallocation\r\n");
            }

            // 释放内存
            sys_free(SYS_MEM_INTERNAL, larger_mem);
            log_i("Memory freed successfully\r\n");
        }
        else
        {
            log_i("Failed to reallocate memory\r\n");
            sys_free(SYS_MEM_INTERNAL, test_mem); // 释放原始内存
        }
    }
    else
    {
        log_i("Failed to allocate memory from SYS_MEM_INTERNAL\r\n");
    }
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(10);

    // 测试不同内存类型的分配
    log_i("Testing allocation from different memory types...\r\n");
    uint8_t *internal_mem = (uint8_t *)sys_malloc(SYS_MEM_INTERNAL, 64);
    uint8_t *external_mem = (uint8_t *)sys_malloc(SYS_MEM_EXTERNAL, 64);
    uint8_t *custom_mem = (uint8_t *)sys_malloc(SYS_MEM_CUSTOM, 64);

    if (internal_mem != NULL)
    {
        log_i("SYS_MEM_INTERNAL allocation successful\r\n");
        sys_free(SYS_MEM_INTERNAL, internal_mem);
    }
    else
    {
        log_i("SYS_MEM_INTERNAL allocation failed\r\n");
    }
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(5);

    if (external_mem != NULL)
    {
        log_i("SYS_MEM_EXTERNAL allocation successful\r\n");
        sys_free(SYS_MEM_EXTERNAL, external_mem);
    }
    else
    {
        log_i("SYS_MEM_EXTERNAL allocation failed\r\n");
    }
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(5);

    if (custom_mem != NULL)
    {
        log_i("SYS_MEM_CUSTOM allocation successful\r\n");
        sys_free(SYS_MEM_CUSTOM, custom_mem);
    }
    else
    {
        log_i("SYS_MEM_CUSTOM allocation failed\r\n");
    }
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(10);
}

// 新增测试函数：测试sys.h中其他未测试的函数
void advanced_sys_test(void)
{
//    log_i("\r\n============================================\r\n");
    log_i("Testing Advanced System Functions...\r\n");
//    log_i("============================================\r\n");
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(10);

    // 测试 sys_delay_us 函数
    log_i("Testing sys_delay_us function...\r\n");
    uint32_t start_us = sys_get_systick_us();
    sys_delay_us(1000); // 延时1000微秒(1毫秒)
    uint32_t end_us = sys_get_systick_us();
    log_i("Delay 1000 us, actually took %u us\r\n", end_us - start_us);
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(10);

    // 测试 malloc/free/realloc 函数
    log_i("Testing sys_malloc, sys_free and sys_realloc functions...\r\n");
    
    // 分配内存
    uint8_t *test_mem = (uint8_t *)sys_malloc(SYS_MEM_INTERNAL, 128);
    if (test_mem != NULL) {
        log_i("Successfully allocated 128 bytes from SYS_MEM_INTERNAL\r\n");
        
        // 检查内存地址是否在正确的区域
        uint32_t addr = (uint32_t)test_mem;
        log_i("SYS_MEM_INTERNAL allocated address: 0x%08X\r\n", addr);
        if (addr >= 0x24000000 && addr < 0x24000000 + 512*1024) {
            log_i("Address is in correct RAM_D1 region (0x24000000 - 0x2407FFFF)\r\n");
        } else {
            log_i("WARNING: Address is NOT in expected RAM_D1 region!\r\n");
        }
        
        // 初始化内存内容
        for (int i = 0; i < 128; i++) {
            test_mem[i] = i;
        }
        
        // 测试 realloc 扩大内存
        uint8_t *larger_mem = (uint8_t *)sys_realloc(SYS_MEM_INTERNAL, test_mem, 256);
        if (larger_mem != NULL) {
            log_i("Successfully reallocated to 256 bytes\r\n");
            
            // 检查重新分配的内存地址
            uint32_t new_addr = (uint32_t)larger_mem;
            log_i("Reallocated address: 0x%08X\r\n", new_addr);
            if (new_addr >= 0x24000000 && new_addr < 0x24000000 + 512*1024) {
                log_i("Reallocated address is in correct RAM_D1 region\r\n");
            } else {
                log_i("WARNING: Reallocated address is NOT in expected RAM_D1 region!\r\n");
            }
            
            // 验证原来的数据是否还在
            int data_valid = 1;
            for (int i = 0; i < 128; i++) {
                if (larger_mem[i] != i) {
                    data_valid = 0;
                    break;
                }
            }
            
            if (data_valid) {
                log_i("Original data preserved after reallocation\r\n");
            } else {
                log_i("Data corruption detected after reallocation\r\n");
            }
            
            // 释放内存
            sys_free(SYS_MEM_INTERNAL, larger_mem);
            log_i("Memory freed successfully\r\n");
        } else {
            log_i("Failed to reallocate memory\r\n");
            sys_free(SYS_MEM_INTERNAL, test_mem); // 释放原始内存
        }
    } else {
        log_i("Failed to allocate memory from SYS_MEM_INTERNAL\r\n");
    }
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(15);
    
    // 测试不同内存类型的分配
    log_i("Testing allocation from different memory types...\r\n");
    uint8_t *internal_mem = (uint8_t *)sys_malloc(SYS_MEM_INTERNAL, 64);
    uint8_t *external_mem = (uint8_t *)sys_malloc(SYS_MEM_EXTERNAL, 64);
    uint8_t *custom_mem = (uint8_t *)sys_malloc(SYS_MEM_CUSTOM, 64);
    
    if (internal_mem != NULL) {
        uint32_t addr = (uint32_t)internal_mem;
        log_i("SYS_MEM_INTERNAL allocation successful at 0x%08X\r\n", addr);
        // 检查地址是否在RAM_D1区域 (0x24000000 - 0x2407FFFF)
        if (addr >= 0x24000000 && addr < 0x24000000 + 512*1024) {
            log_i("SYS_MEM_INTERNAL address is in correct RAM_D1 region\r\n");
        } else {
            log_i("WARNING: SYS_MEM_INTERNAL address is NOT in expected RAM_D1 region!\r\n");
        }
        sys_free(SYS_MEM_INTERNAL, internal_mem);
    } else {
        log_i("SYS_MEM_INTERNAL allocation failed\r\n");
    }
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(5);
    
    if (external_mem != NULL) {
        uint32_t addr = (uint32_t)external_mem;
        log_i("SYS_MEM_EXTERNAL allocation successful at 0x%08X\r\n", addr);
        // 检查地址是否在SDRAM区域 (0xC0000000 - 0xC1FFFFFF)
        if (addr >= 0xC0000000 && addr < 0xC0000000 + 32*1024*1024) {
            log_i("SYS_MEM_EXTERNAL address is in correct SDRAM region\r\n");
        } else {
            log_i("WARNING: SYS_MEM_EXTERNAL address is NOT in expected SDRAM region!\r\n");
        }
        sys_free(SYS_MEM_EXTERNAL, external_mem);
    } else {
        log_i("SYS_MEM_EXTERNAL allocation failed\r\n");
    }
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(5);
    
    if (custom_mem != NULL) {
        uint32_t addr = (uint32_t)custom_mem;
        log_i("SYS_MEM_CUSTOM allocation successful at 0x%08X\r\n", addr);
        // 检查地址是否在DTCM区域 (0x20000000 - 0x2001FFFF)
        if (addr >= 0x20000000 && addr < 0x20000000 + 128*1024) {
            log_i("SYS_MEM_CUSTOM address is in correct DTCM region\r\n");
        } else {
            log_i("WARNING: SYS_MEM_CUSTOM address is NOT in expected DTCM region!\r\n");
        }
        sys_free(SYS_MEM_CUSTOM, custom_mem);
    } else {
        log_i("SYS_MEM_CUSTOM allocation failed\r\n");
    }
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(10);
}

void sys_test(void)
{
    sys_bind_ops(SysFactory_GetOps());
    sys_core_init();

    // test project
    base_test();
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(20);
    
    RamSpeed_test();
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(20);
    
    BaseRead_test();
    
    // 添加延时以防止日志输出缓冲区溢出
    sys_delay_ms(20);
}

#endif
