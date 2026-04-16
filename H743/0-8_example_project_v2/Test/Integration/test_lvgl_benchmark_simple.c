#include "test_config.h"
#if ENABLE_TEST_LVGL_BENCHMARK

#include "lvgl.h"
#include "test_framework.h"

// 前向声明显示刷新控制函数
extern void disp_enable_update(void);
extern void disp_disable_update(void);

static bool benchmark_finished = false;

static void on_benchmark_finished(void)
{
    benchmark_finished = true;
    disp_enable_update();
    log_i("Simple LVGL Benchmark finished!");
}

static void test_lvgl_benchmark_simple_setup(void)
{
    log_i("Simple LVGL Benchmark Test Setup");
    
    if (!lv_is_initialized()) {
        log_e("LVGL is not initialized!");
        return;
    }
    
    // 启用LCD刷新以便看到效果
    disp_enable_update();
    
    // 只运行一个简单的场景（场景编号可以从0开始尝试）
    log_i("Running single benchmark scene (scene 0)...");
    lv_demo_benchmark_run_scene(0);
    
    benchmark_finished = true; // 简单场景立即完成
    log_i("Single scene completed.");
}

static void test_lvgl_benchmark_simple_loop(void)
{
    // 简单测试不需要循环逻辑
}

static void test_lvgl_benchmark_simple_teardown(void)
{
    log_i("Simple LVGL Benchmark cleanup completed.");
}

REGISTER_TEST(LVGL_BENCHMARK_SIMPLE, "Run single LVGL benchmark scene", 
              test_lvgl_benchmark_simple_setup, test_lvgl_benchmark_simple_loop, 
              test_lvgl_benchmark_simple_teardown);

#endif