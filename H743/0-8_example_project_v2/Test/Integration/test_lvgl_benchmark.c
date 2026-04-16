#include "test_config.h"
#if ENABLE_TEST_LVGL_BENCHMARK

#include "lvgl.h"
#include "test_framework.h"

// LVGL benchmark API通过lvgl.h自动包含（通过lv_demos.h）

// 前向声明显示刷新控制函数
extern void disp_enable_update(void);
extern void disp_disable_update(void);

static bool benchmark_finished = false;
static uint32_t benchmark_start_time = 0;

// 配置选项：是否禁用LCD刷新进行纯性能测试
#define BENCHMARK_DISABLE_LCD_FLUSH 0  // 设置为0可以看到测试过程，设置为1进行纯性能测试

static void on_benchmark_finished(void)
{
    benchmark_finished = true;
    if (BENCHMARK_DISABLE_LCD_FLUSH) {
        disp_enable_update();
    }
    log_i("LVGL Benchmark finished! Duration: %lu ms", 
          sys_get_systick_ms() - benchmark_start_time);
}

static void test_lvgl_benchmark_setup(void)
{
    log_i("LVGL Benchmark Test Setup: Initializing benchmark environment.");
    
    // 检查LVGL是否已初始化
    if (!lv_is_initialized()) {
        log_e("LVGL is not initialized!");
        return;
    }
    
    // 记录开始时间
    benchmark_start_time = sys_get_systick_ms();
    
    // 根据配置决定是否禁用LCD刷新
    if (BENCHMARK_DISABLE_LCD_FLUSH) {
        log_i("Disabling LCD flush for pure performance measurement...");
        disp_disable_update();
    } else {
        log_i("Keeping LCD flush enabled - you will see the benchmark running...");
        disp_enable_update();
    }
    
    // 注册完成回调
    lv_demo_benchmark_set_finished_cb(on_benchmark_finished);
    
    // 设置最高测试速度
    lv_demo_benchmark_set_max_speed(true);
    
    log_i("Starting LVGL benchmark test...");
    // 启动benchmark测试
    lv_demo_benchmark();
    
    benchmark_finished = false;
    log_i("LVGL benchmark started successfully.");
}

static void test_lvgl_benchmark_loop(void)
{
    static uint32_t last_log_time = 0;
    
    // 如果benchmark已完成，可以执行其他操作或等待用户输入
    if (benchmark_finished) {
        // 可以在这里添加结果处理逻辑
        static bool result_shown = false;
        if (!result_shown) {
            log_i("LVGL Benchmark results are displayed on screen.");
            result_shown = true;
        }
    } else {
        // benchmark还在运行中，定期输出状态
        if (sys_get_systick_ms() - last_log_time > 5000) {  // 每5秒输出一次
            last_log_time = sys_get_systick_ms();
            log_d("LVGL Benchmark still running... (elapsed: %lu ms)", 
                  sys_get_systick_ms() - benchmark_start_time);
        }
    }
    // 让LVGL继续处理benchmark
}

static void test_lvgl_benchmark_teardown(void)
{
    log_i("LVGL Benchmark Test Teardown: Cleaning up.");
    
    // 如果benchmark还在运行，关闭它
    if (!benchmark_finished) {
        log_w("Benchmark was not finished, forcing close...");
        lv_demo_benchmark_close();
        if (BENCHMARK_DISABLE_LCD_FLUSH) {
            disp_enable_update();
        }
    }
    
    benchmark_finished = false;
    log_i("LVGL Benchmark cleanup completed.");
}

REGISTER_TEST(LVGL_BENCHMARK, "Run LVGL performance benchmark test", 
              test_lvgl_benchmark_setup, test_lvgl_benchmark_loop, 
              test_lvgl_benchmark_teardown);

#endif