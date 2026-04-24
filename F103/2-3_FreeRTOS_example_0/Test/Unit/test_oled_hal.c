/*
 * test_oled_hal.c
 *
 *  Created on: Mar 6, 2026
 *      Author: 12114
 */

#include "test_config.h"

#if ENABLE_TEST_OLED_HAL
#include "Sys.h"
#include "dev_map.h"
#include "oled/oled.h"
#include "oled/font.h"
#include "test_framework.h"
#include <stdio.h>


static void test_oled_hal_setup(void) {
  OLED_Init();
  log_i("OLED Test Setup: OLED initialized.");
}

static void test_oled_hal_loop(void) {
    for (uint8_t i = 0; i < 256; i++)
    {
      OLED_NewFrame();
      OLED_DrawImage((128 - (bilibiliImg.w)) / 2, 0, &bilibiliImg, OLED_COLOR_NORMAL);
      OLED_PrintString(128 - i, 64 - 16, "波特律动hello", &font16x16, OLED_COLOR_NORMAL);
      OLED_ShowFrame();
    }
    sys_delay_ms(50);
    log_i(".");
}

static void test_oled_hal_teardown(void) {
	log_i("OLED Test Teardown: Clearing screen.");
}

REGISTER_TEST(OLED, "Display text and graphics on OLED", test_oled_hal_setup, test_oled_hal_loop,
              test_oled_hal_teardown);

#endif

