#include "test_config.h"

#if ENABLE_TEST_ENCODER
#include "dev_map.h"
#include "absolute_encoder_driver.h"
#include "factory_inc.h"
#include "test_framework.h"
#include <stdio.h>

static absolute_encoder_driver_t *encoder_dev;

static void test_encoder_setup(void) {
    // 使用工厂函数获取编码器设备实例（使用M0电机的编码器）
    encoder_dev = absolute_encoder_driver_get(ENCODER_ID_M0);
    if (encoder_dev == NULL) {
        log_e("Failed to get ENCODER_ID_M0");
        return;
    }
    log_i("Encoder Test Setup: Ready.");
    
    // 初始读取一次状态用于调试
    uint16_t raw_angle = ENCODER_GET_RAW(encoder_dev);
    uint8_t magnet_status = ENCODER_GET_STATUS(encoder_dev);
    log_i("Initial Read - Raw: %d, Status: %s", 
          raw_angle, magnet_status ? "OK" : "MISSING");
}

static void test_encoder_loop(void) {
    static uint32_t last_tick = 0;
    char str_buf[64];

    if (sys_get_systick_ms() - last_tick >= 500) { // 每500ms读取一次数据
        last_tick = sys_get_systick_ms();
        
        // 获取原始角度值 (0-4095)
        uint16_t raw_angle = ENCODER_GET_RAW(encoder_dev);
        // 获取弧度值 (0 - 2PI)
        float angle_rad = ENCODER_GET_ANGLE(encoder_dev);
        // 获取磁铁状态
        uint8_t magnet_status = ENCODER_GET_STATUS(encoder_dev);
        
        // 转换为角度制 (0-360度)
        float angle_deg = angle_rad * 180.0f / 3.14159265358979323846f;
        
        // 打印编码器信息
        sprintf(str_buf, "Raw: %4d, Deg: %6.1f°, Rad: %5.3f", 
                raw_angle, angle_deg, angle_rad);
        log_i("%s", str_buf);
        
        // 打印磁铁状态
        if (magnet_status) {
            log_i("Magnet Status: OK");
        } else {
            log_w("Magnet Status: MISSING!");
            // 添加额外的调试信息
            if (raw_angle == 0) {
                log_w("Warning: Raw angle is 0, possible I2C communication failure!");
            }
        }
        
        // 添加分隔线便于阅读
        log_i("------------------------");
    }
}

static void test_encoder_teardown(void) {
    // 编码器不需要特殊的清理操作
    log_i("Encoder Test Teardown: Complete.");
}

REGISTER_TEST(ENCODER, "AS5600 Absolute Encoder Test", test_encoder_setup, test_encoder_loop,
              test_encoder_teardown);

#endif