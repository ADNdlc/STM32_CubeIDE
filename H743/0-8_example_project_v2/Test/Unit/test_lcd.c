#include "test_config.h"
#if ENABLE_TEST_LCD
#include "dev_map.h"
#include "lcd_screen_factory.h"
#include "test_framework.h"

static void test_lcd_setup(void) {
  log_i("LCD Test Setup: Initializing LCD.");
  
}

static void test_lcd_loop(void) {}

static void test_lcd_teardown(void) {
  log_i("LCD Test Teardown: Cleaning up.");
}

REGISTER_TEST(LCD, "Test the basic functions of the LCD", test_lcd_setup,
              test_lcd_loop, test_lcd_teardown);

#endif
