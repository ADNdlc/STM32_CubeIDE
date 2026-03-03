#include "test_config.h"
#if ENABLE_TEST_LCD
#include "Sys.h"
#include "MemPool.h"
#include "dev_map.h"
#include "lcd_screen_factory.h"
#include "test_framework.h"

#define LCD_TEST_ASSERT(cond)                                                  \
  if (!(cond)) {                                                               \
    log_e("Assertion failed: %s", #cond);                                      \
    Stop_Current_Test();                                                       \
    return;                                                                    \
  }
#define LCD_TEST_ASSERT_NOT_NULL(ptr)                                          \
  if ((ptr) == NULL) {                                                         \
    log_e("Assertion failed: %s is NULL", #ptr);                               \
    Stop_Current_Test();                                                       \
    return;                                                                    \
  }
#define LCD_TEST_ASSERT_EQUAL(expected, actual)                                \
  if ((expected) != (actual)) {                                                \
    log_e("Assertion failed: Expected 0x%X, got 0x%X",                         \
          (unsigned int)(expected), (unsigned int)(actual));                   \
    Stop_Current_Test();                                                       \
    return;                                                                    \
  }

static lcd_driver_t *lcd = NULL;
static uint8_t* buffer1= NULL;
static uint8_t* buffer2= NULL;

static void test_lcd_setup(void) {
  log_i("LCD Test Setup: Initializing LCD.");
  buffer1 = sys_malloc(SYS_MEM_EXTERNAL, lcd->info.width * lcd->info.height * 2);
  if (!buffer1) {
    log_e("Failed to allocate buffer1");
    Stop_Current_Test();
    return;
  }

  buffer2 = LCD_GET_ACT_BUFFER(lcd);
  if (!buffer2) {
    log_e("Failed to get buffer2");
    Stop_Current_Test();
    return;
  }
  log_i("buffer1: %p, buffer2: %p", buffer1, buffer2);

  lcd = lcd_screen_factory_create(LCD_ID_UI);
  LCD_TEST_ASSERT_NOT_NULL(lcd);

  if (lcd) {
    int res = LCD_INIT(lcd);
    LCD_TEST_ASSERT_EQUAL(0, res);

    // 打开显示
    LCD_DISPLAY_ON(lcd);
    log_i("LCD Info: %dx%d, Format: %d", lcd->info.width, lcd->info.height,
          lcd->info.format);
  }
}

static void test_lcd_loop(void) {
  if (!lcd)
    return;

  log_i("Running LCD basic tests...");

  // 1. 测试清屏 (填充红色)
  // 注意：0xF800 是 RGB565 的红色
  int res = LCD_FILL(lcd, 0, 0, lcd->info.width, lcd->info.height, 0xF800);
  LCD_TEST_ASSERT_EQUAL(0, res);
  sys_delay_ms(500);

  // 2. 测试读写点
  uint32_t test_color = 0x07E0; // Green
  LCD_DRAW_POINT(lcd, 100, 100, test_color);
  uint32_t read_color = LCD_READ_POINT(lcd, 100, 100);
  log_i("Read color: 0x%04X", read_color);
  LCD_TEST_ASSERT_EQUAL(test_color, (uint16_t)read_color);

  // 3. 测试填充蓝色矩形
  res = LCD_FILL(lcd, 200, 200, 100, 100, 0x001F); // Blue
  LCD_TEST_ASSERT_EQUAL(0, res);
  sys_delay_ms(500);

  // Clear screen for animation start
  res = LCD_FILL(lcd, 0, 0, lcd->info.width, lcd->info.height, 0x0000);
  LCD_TEST_ASSERT_EQUAL(0, res);

  // Bouncing Box Animation Loop - Visual Check
  int16_t x = 0, y = 0;
  int16_t dx = 10, dy = 10;
  uint16_t box_w = 100, box_h = 100;
  uint32_t color = 0xF800; // Red
  uint32_t time = sys_get_systick_ms();
  while (time + 10000 > sys_get_systick_ms()) {
    LCD_FILL(lcd, 0, 0, lcd->info.width, lcd->info.height, 0x0000); // 清除上一个显示

    LCD_FILL(lcd, x, y, box_w, box_h, color);
    x += dx;
    y += dy;
    // Collision detection
    if (x < 0 || x + box_w >= lcd->info.width) {
      dx = -dx;
      x += dx;
      color = (color == 0xF800) ? 0x07E0 : 0xF800; // Toggle Red/Green
    }
    if (y < 0 || y + box_h >= lcd->info.height) {
      dy = -dy;
      y += dy;
      color = (color == 0xF800) ? 0x001F : 0xF800; // Toggle to Blue if red
    }
    sys_delay_ms(50);
  }

  log_i("LCD basic tests passed.");
  // 停止测试循环
  Stop_Current_Test();
}

static void test_lcd_teardown(void) {
  log_i("LCD Test Teardown: Clearing screen and turning off display.");
  if (lcd) {
    // 强制清屏为黑色，避免残留
    LCD_FILL(lcd, 0, 0, lcd->info.width, lcd->info.height, 0x0000);
    sys_delay_ms(100);
    LCD_DISPLAY_OFF(lcd);
  }
}

REGISTER_TEST(LCD, "Test the basic functions of the LCD", test_lcd_setup,
              test_lcd_loop, test_lcd_teardown);

#endif
