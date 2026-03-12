#include "test_config.h"
#if ENABLE_TEST_LCD
#include "MemPool.h"
#include "Sys.h"
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

extern lcd_driver_t *lcd;
static void *buffer1 = NULL;
static void *buffer2 = NULL;

static void test_lcd_setup(void) {
  log_i("LCD Test Setup: Initializing LCD.");

  lcd = lcd_screen_factory_create(LCD_ID_UI);
  LCD_TEST_ASSERT_NOT_NULL(lcd);

  // 1. 获取默认缓冲区 (板级配置好的 buffer1)
  buffer1 = LCD_GET_ACT_BUFFER(lcd);
  buffer2 = LCD_GET_BACK_BUFFER(lcd);
  if (buffer1 == NULL || buffer2 == NULL) {
    log_e("LCD memory configuration failed");
    Stop_Current_Test();
  }
  log_i("Double Buffering: Front=%p, Back=%p", buffer1, buffer2);

  int res = LCD_INIT(lcd);
  LCD_TEST_ASSERT_EQUAL(0, res);

  LCD_DISPLAY_ON(lcd);
  log_i("LCD Info: %dx%d, Format: %d", lcd->info.width, lcd->info.height,
        lcd->info.format);
}

static volatile int swap_count = 0;
static volatile int fill_count = 0;

static void on_lcd_swap_done(void *user_data) { swap_count++; }

static void on_lcd_fill_done(void *user_data) { fill_count++; }

static void test_lcd_loop(void) {
  if (!lcd)
    return;

  log_i("Running LCD basic tests...");

  // 1. 测试清屏 (填充红色)
  int res = LCD_FILL(lcd, 0, 0, lcd->info.width, lcd->info.height, 0xF800);
  LCD_TEST_ASSERT_EQUAL(0, res);
  LCD_SWAP_BUFFER(lcd);
  LCD_WAIT_SWAP(lcd);
  sys_delay_ms(500);

  // 2. 测试读写点
  uint32_t test_color = 0x07E0; // Green
  LCD_DRAW_POINT(lcd, 100, 100, test_color);
  uint32_t read_color = LCD_READ_POINT(lcd, 100, 100);
  log_i("Read color: 0x%04X", read_color);
  LCD_TEST_ASSERT_EQUAL(test_color, (uint16_t)read_color);

  // 3. 测试异步回调模式
  log_i("Testing Asynchronous Callback Mode...");
  LCD_SET_FILL_CB(lcd, on_lcd_fill_done, NULL);
  LCD_SET_SWAP_CB(lcd, on_lcd_swap_done, NULL);

  fill_count = 0;
  res = LCD_FILL(lcd, 0, 0, lcd->info.width, lcd->info.height, 0x001F); // Blue
  LCD_TEST_ASSERT_EQUAL(0, res);
  // 等待填充中断
  uint32_t timeout = sys_get_systick_ms() + 1000;
  while (fill_count == 0 && sys_get_systick_ms() < timeout) {
  }
  LCD_TEST_ASSERT(fill_count > 0);
  log_i("Async Fill Callback received.");

  swap_count = 0;
  LCD_SWAP_BUFFER(lcd);
  // 等待垂直消隐切换中断
  timeout = sys_get_systick_ms() + 1000;
  while (swap_count == 0 && sys_get_systick_ms() < timeout) {
  }
  LCD_TEST_ASSERT(swap_count > 0);
  log_i("Async Swap Callback received.");

  // 清除回调，回到同步模式继续动画测试
  LCD_SET_FILL_CB(lcd, NULL, NULL);
  LCD_SET_SWAP_CB(lcd, NULL, NULL);

  // Bouncing Box Animation Loop - Double Buffered
  int16_t x = 0, y = 0;
  int16_t dx = 5, dy = 5;
  uint16_t box_w = 100, box_h = 100;
  uint32_t color = 0xF800; // Red

  uint32_t end_time = sys_get_systick_ms() + 5000;

  while (sys_get_systick_ms() < end_time) {
    LCD_FILL(lcd, 0, 0, lcd->info.width, lcd->info.height, 0x0000); // 清除后台
    LCD_FILL(lcd, x, y, box_w, box_h, color); // 在后台绘制新帧

    LCD_SWAP_BUFFER(lcd);
    LCD_WAIT_SWAP(lcd); // 同步等待 VSYNC

    // 计算下一帧坐标
    x += dx;
    y += dy;
    if (x < 0 || x + box_w >= lcd->info.width) {
      dx = -dx;
      x += dx;
      color = (color == 0xF800) ? 0x07E0 : 0xF800;
    }
    if (y < 0 || y + box_h >= lcd->info.height) {
      dy = -dy;
      y += dy;
      color = (color == 0xF800) ? 0x001F : 0xF800;
    }
    sys_delay_ms(10); // 控制帧率
  }

  log_i("LCD tests passed.");
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
