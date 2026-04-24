#include "test_config.h"

#if ENABLE_APP_POWER_MONITOR
#include "PowerMonitor_driver.h"
#include "PowerMonitor_factory.h"
#include "oled_driver.h"
#include "OLED_factory.h"
#include "oled_font.h"
#include "test_framework.h"
#include <stdio.h>

#define FULL_CAPACITY_MAH 2000.0f

static PowerMonitor_driver_t *pm_dev;
static oled_device_t *oled_dev;

static void test_power_app_setup(void) {
	elog_set_filter_tag_lvl("INA219", ELOG_FILTER_LVL_SILENT);
    // 获取设备句柄
    pm_dev = PowerMonitor_factory_get(POWER_MONITOR_ID_MAIN);
    oled_dev = OLED_Factory_Get(OLED_ID_MAIN);

    if (pm_dev == NULL || oled_dev == NULL) {
        log_e("Failed to get PowerMonitor or OLED device");
        return;
    }

    // 初始化 OLED
    OLED_Init(oled_dev);
    OLED_Clear(oled_dev, OLED_COLOR_NORMAL);
    OLED_PrintString(oled_dev, 0, 0, "Power Monitor", &font16x16, OLED_COLOR_NORMAL);
    OLED_ReFresh(oled_dev);
    
    log_i("Power App Setup: Initialized.");
}

static void test_power_app_loop(void) {
    static uint32_t last_tick = 0;
    Power_Instant_Data_t instant;
    Power_Accumulated_Data_t accumulated;
    char buf[32];

    if (sys_get_systick_ms() - last_tick >= 500) {
        last_tick = sys_get_systick_ms();

        if (PM_READ_INSTANT(pm_dev, &instant) == 0 && 
            PM_READ_ACCUMULATED(pm_dev, &accumulated) == 0) {
            
            OLED_Clear(oled_dev, OLED_COLOR_NORMAL);
            
            // 1. 电压
            sprintf(buf, "V: %.2f V", instant.voltage_mV / 1000.0f);
            OLED_PrintString(oled_dev, 0, 0, buf, &font16x16, OLED_COLOR_NORMAL);
            
            // 2. 电流 & 功率
            sprintf(buf, "I:%.1fmA P:%.0fmW", instant.current_mA, instant.power_mW);
            OLED_PrintASCIIString(oled_dev, 0, 16, buf, &afont12x6, OLED_COLOR_NORMAL);
            
            // 3. 已消耗能量 (mAh)
            sprintf(buf, "Used: %.1f mAh", (float)accumulated.charge_mAh);
            OLED_PrintString(oled_dev, 0, 32, buf, &font16x16, OLED_COLOR_NORMAL);
            
            // 4. 剩余电量 & 进度条
            float remaining = FULL_CAPACITY_MAH - (float)accumulated.charge_mAh;
            if (remaining < 0) remaining = 0;
            float percent = (remaining / FULL_CAPACITY_MAH) * 100.0f;
            
            sprintf(buf, "Rem: %.0f%%", percent);
            OLED_PrintASCIIString(oled_dev, 0, 52, buf, &afont12x6, OLED_COLOR_NORMAL);
            
            // 绘制进度条框
            uint8_t bar_x = 64;
            uint8_t bar_y = 54;
            uint8_t bar_w = 60;
            uint8_t bar_h = 8;
            OLED_DrawRectangle(oled_dev, bar_x, bar_y, bar_w, bar_h, OLED_COLOR_NORMAL);
            
            // 绘制填充部分
            uint8_t fill_w = (uint8_t)((percent / 100.0f) * (bar_w - 2));
            if (fill_w > 0) {
                OLED_DrawFilledRectangle(oled_dev, bar_x + 1, bar_y + 1, fill_w, bar_h - 2, OLED_COLOR_NORMAL);
            }
            
            OLED_ReFresh(oled_dev);
        }
    }
}

static void test_power_app_teardown(void) {
    if (oled_dev) {
        OLED_Clear(oled_dev, OLED_COLOR_NORMAL);
        OLED_ReFresh(oled_dev);
    }
}

REGISTER_TEST(PowerApp, "Display Real-time Power Stats on OLED", 
              test_power_app_setup, test_power_app_loop, test_power_app_teardown);

#endif
