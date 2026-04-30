#include "test_config.h"

#if ENABLE_TEST_ENCODER_SPEED
#include "dev_map.h"
#include "absolute_encoder_driver.h"
#include "factory_inc.h"
#include "test_framework.h"
#include <stdio.h>

static absolute_encoder_driver_t *encoder_dev;
static uint32_t current_interval_us = 1000;
static uint32_t min_successful_interval = 1000;
static int test_round = 0;
static bool test_completed = false;

static void test_encoder_speed_setup(void) {
    // 使用工厂函数获取编码器设备实例（使用M0电机的编码器）
    encoder_dev = absolute_encoder_driver_get(ENCODER_ID_M0);
    if (encoder_dev == NULL) {
        log_e("Failed to get ENCODER_ID_M0");
        return;
    }
    
    // 重置测试状态
    current_interval_us = 1000;
    min_successful_interval = 1000;
    test_round = 0;
    test_completed = false;
    
    log_i("Encoder Speed Test Setup: Ready. Starting with %dms interval.", current_interval_us);
}

static void test_encoder_speed_loop(void) {
    if (test_completed) {
        return;
    }
    
    static uint32_t last_test_time = 0;
    uint32_t current_time = sys_get_systick_us();
    
    // 检查是否到了下一轮测试的时间
    if (current_time - last_test_time >= current_interval_us) {
        // 执行20次连续读取
        bool all_reads_successful = true;
        for (int i = 0; i < 20; i++) {
            uint16_t raw_angle = ENCODER_GET_RAW(encoder_dev);
            uint8_t magnet_status = ENCODER_GET_STATUS(encoder_dev);
            
            // 检查是否发生总线错误（raw_angle为0且magnet_status为0通常表示通信失败）
            if (raw_angle == 0 && magnet_status == 0) {
                all_reads_successful = false;
                break;
            }
        }
        
        if (all_reads_successful) {
            // 本轮测试成功，记录当前间隔时间为最小成功时间
            min_successful_interval = current_interval_us;
            log_i("Round %d: Success with %dus interval", test_round + 1, current_interval_us);
            
            // 减少间隔时间进行下一轮测试
            if (current_interval_us > 0) {
                current_interval_us-=100;
                test_round++;
                last_test_time = current_time;
            } else {
                // 测试完成
                log_i("Test completed! Minimum successful interval: %dus", min_successful_interval);
                test_completed = true;
            }
        } else {
            // 发生总线错误，停止测试
            log_e("Round %d: Bus error detected at %dms interval!", test_round + 1, current_interval_us);
            log_i("Minimum successful interval was: %dms", min_successful_interval);
            test_completed = true;
        }
    }
}

static void test_encoder_speed_teardown(void) {
    log_i("Encoder Speed Test Teardown: Complete.");
    log_i("Final result - Minimum successful interval: %dus", min_successful_interval);
}

REGISTER_TEST(ENCODER_SPEED, "AS5600 Encoder Speed Test", test_encoder_speed_setup, test_encoder_speed_loop,
              test_encoder_speed_teardown);

#endif