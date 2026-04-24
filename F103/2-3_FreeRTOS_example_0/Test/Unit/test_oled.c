#include "test_config.h"

#if ENABLE_TEST_OLED
#include "dev_map.h"
#include "oled_driver.h"
#include "OLED_factory.h"
#include "oled_font.h"
#include "test_framework.h"
#include <stdio.h>

static oled_device_t *oled_dev;

static void test_oled_setup(void) {
    oled_dev = OLED_Factory_Get(OLED_ID_MAIN);
    if (oled_dev == NULL) {
        log_e("OLED_ID_MAIN not found");
        return;
    }
    OLED_Init(oled_dev);
    log_i("OLED Test Setup: Ready.");
}

static void test_oled_loop(void) {
    static uint32_t last_tick = 0;
    static uint8_t page = 0;
    static uint8_t frame_cnt = 0;
    char str_buf[32];

    if (sys_get_systick_ms() - last_tick >= 2000) { // 每2秒切换一页
        last_tick = sys_get_systick_ms();
        
        OLED_Clear(oled_dev, OLED_COLOR_NORMAL);
        
        switch (page) {
            case 0: // Page 0: Fonts & Text
                OLED_PrintString(oled_dev, 0, 0, "Page 0: Fonts", &font16x16, OLED_COLOR_NORMAL);
                OLED_PrintString(oled_dev, 0, 20, "波特律动", &font16x16, OLED_COLOR_NORMAL);
                OLED_PrintString(oled_dev, 0, 40, "ASCII: Hello!", &font16x16, OLED_COLOR_NORMAL);
                break;
                
            case 1: // Page 1: Basic Graphics
                OLED_PrintString(oled_dev, 0, 0, "Page 1: Graphics", &font16x16, OLED_COLOR_NORMAL);
                OLED_DrawLine(oled_dev, 0, 16, 127, 16, OLED_COLOR_NORMAL);
                OLED_DrawCircle(oled_dev, 32, 40, 15, OLED_COLOR_NORMAL);
                OLED_DrawFilledCircle(oled_dev, 96, 40, 10, OLED_COLOR_NORMAL);
                OLED_DrawRectangle(oled_dev, 60, 30, 20, 20, OLED_COLOR_NORMAL);
                break;
                
            case 2: // Page 2: Advanced Graphics
                OLED_PrintString(oled_dev, 0, 0, "Page 2: Shapes", &font16x16, OLED_COLOR_NORMAL);
                OLED_DrawTriangle(oled_dev, 10, 20, 40, 20, 25, 50, OLED_COLOR_NORMAL);
                OLED_DrawFilledTriangle(oled_dev, 60, 20, 90, 20, 75, 50, OLED_COLOR_NORMAL);
                OLED_DrawEllipse(oled_dev, 110, 40, 15, 10, OLED_COLOR_NORMAL);
                break;
                
            case 3: // Page 3: Images & Inversion
                OLED_PrintString(oled_dev, 0, 0, "Page 3: Image", &font16x16, OLED_COLOR_NORMAL);
                // 绘制毕哩毕哩图标 (51x48)
                OLED_DrawImage(oled_dev, 38, 16, &bilibiliImg, OLED_COLOR_NORMAL);
                break;

            case 4: // Page 4: Full Refresh Test
                sprintf(str_buf, "Frame: %d", frame_cnt++);
                OLED_PrintString(oled_dev, 0, 0, "Page 4: Dynamic", &font16x16, OLED_COLOR_NORMAL);
                OLED_PrintString(oled_dev, 0, 24, str_buf, &font16x16, OLED_COLOR_NORMAL);
                OLED_DrawFilledRectangle(oled_dev, 0, 48, (frame_cnt % 128), 10, OLED_COLOR_NORMAL);
                break;
        }
        
        OLED_ReFresh(oled_dev);
        log_i("Displayed Page %d", page);
        
        page = (page + 1) % 5;
    }
}

static void test_oled_teardown(void) {
    if (oled_dev) {
        OLED_Clear(oled_dev, OLED_COLOR_NORMAL);
        OLED_ReFresh(oled_dev);
    }
}

REGISTER_TEST(OLED, "Full feature multi-page OLED test", test_oled_setup, test_oled_loop,
              test_oled_teardown);

#endif
