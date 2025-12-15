/*
 * lcd_test.c
 *
 *  Created on: Dec 10, 2025
 *      Author: 12114
 */

#include "all_tests_config.h"

#if _lcd_test_
#include "device_mapping.h"
#include "elog.h"
#include "lcd_factory.h"
#include "lcd_hal/lcd_hal.h"
#include "sys.h"
#include <stdint.h>
#include <string.h>

// 800x480 resolution
static uint16_t display_buffer[800][480] __attribute__((section(".sdram_section"), aligned(16)));
static uint16_t draw_buffer[800][480] __attribute__((section(".sdram_section"), aligned(16)));
#define LCD_WIDTH 800
#define LCD_HEIGHT 480
#define BUFFER_SIZE (LCD_WIDTH * LCD_HEIGHT * 2)

void lcd_test_run(void) {
  log_i("LCD Test Start");
  lcd_driver_t *lcd_driver = lcd_driver_get(LCD_MAIN, LCD_WIDTH, LCD_HEIGHT);
  if (lcd_driver == NULL) {
    log_e("LCD Driver get failed");
    return;
  }

  // Allocate buffers in external memory (SDRAM)
//  uint16_t *display_buf = (uint16_t *)sys_malloc(SYS_MEM_EXTERNAL, BUFFER_SIZE);
//  uint16_t *draw_buf = (uint16_t *)sys_malloc(SYS_MEM_EXTERNAL, BUFFER_SIZE);
  uint16_t *display_buf = display_buffer;
  uint16_t *draw_buf = draw_buffer;

  if (!display_buf || !draw_buf) {
    log_e("Buffer allocation failed");
    if (display_buf)
      sys_free(SYS_MEM_EXTERNAL, display_buf);
    if (draw_buf)
      sys_free(SYS_MEM_EXTERNAL, draw_buf);
    return;
  }

  // Initialize buffers to black
  memset(display_buf, 0, BUFFER_SIZE);
  memset(draw_buf, 0, BUFFER_SIZE);

  lcd_hal_t *lcd_hal = lcd_hal_create(lcd_driver, draw_buf, display_buf);
  if (lcd_hal == NULL) {
    log_e("LCD HAL create failed");
    sys_free(SYS_MEM_EXTERNAL, display_buf);
    sys_free(SYS_MEM_EXTERNAL, draw_buf);
    return;
  }

  log_i("LCD HAL create Success");

  // --- Functional Tests ---

  // 1. Buffer Access Test
  uint16_t *curr_draw = lcd_hal_get_drawbuf(lcd_hal);
  uint16_t *curr_disp = lcd_hal_get_displaybuf(lcd_hal);
  log_i("Draw Buf: 0x%p, Disp Buf: 0x%p", curr_draw, curr_disp);

  if (curr_draw != draw_buf || curr_disp != display_buf) {
    log_e("Buffer mismatch! Expected Draw: 0x%p, Disp: 0x%p", draw_buf,
          display_buf);
  }

  // Test set functions (Swap temporarily)
  lcd_hal_set_drawbuf(lcd_hal, display_buf);
  if (lcd_hal_get_drawbuf(lcd_hal) != display_buf) {
    log_e("lcd_hal_set_drawbuf failed");
  } else {
    log_i("lcd_hal_set_drawbuf functionality verified");
  }
  lcd_hal_set_drawbuf(lcd_hal, draw_buf); // Restore

  // 2. Clear Screen
  lcd_hal_fill_rect(lcd_hal, 0, 0, LCD_WIDTH, LCD_HEIGHT, 0x0000);
  log_i("Screen Cleared");

  // 3. Point Drawing and Reading Test
  uint16_t test_x = 100, test_y = 100;
  uint32_t test_color = 0xF800; // Red
  lcd_hal_draw_point(lcd_hal, test_x, test_y, test_color);
  uint32_t read_color = lcd_hal_read_point(lcd_hal, test_x, test_y);

  if ((uint16_t)read_color != (uint16_t)test_color) {
    log_e("Point Test Failed: Expected 0x%04X, Got 0x%04X At (%d,%d)",
          test_color, read_color, test_x, test_y);
  } else {
    log_i("Point Test Passed");
  }

  // 4. Fill Rect Test
  uint16_t rect_x = 200, rect_y = 200, rect_w = 50, rect_h = 50;
  uint32_t rect_color = 0x07E0; // Green
  lcd_hal_fill_rect(lcd_hal, rect_x, rect_y, rect_w, rect_h, rect_color);

  read_color = lcd_hal_read_point(lcd_hal, rect_x + 10, rect_y + 10);
  if ((uint16_t)read_color != (uint16_t)rect_color) {
    log_e("Fill Rect Failed: Expected 0x%04X, Got 0x%04X inside rect",
          rect_color, read_color);
  } else {
    log_i("Fill Rect Test Passed");
  }

  // 5. Bitmap Drawing Test
  // Create a 2x2 bitmap: White, Black, Black, White
  uint16_t bitmap[4] = {0xFFFF, 0x0000, 0x0000, 0xFFFF};
  lcd_hal_draw_bitmap(lcd_hal, 300, 300, 2, 2, bitmap);

  uint16_t p00 = lcd_hal_read_point(lcd_hal, 300, 300);
  uint16_t p01 = lcd_hal_read_point(lcd_hal, 301, 300); // x+1
  uint16_t p10 = lcd_hal_read_point(lcd_hal, 300, 301); // y+1
  uint16_t p11 = lcd_hal_read_point(lcd_hal, 301, 301);

  if (p00 == 0xFFFF && p01 == 0x0000 && p10 == 0x0000 && p11 == 0xFFFF) {
    log_i("Bitmap Test Passed");
  } else {
    log_e("Bitmap Test Failed: Read %04X %04X %04X %04X", p00, p01, p10, p11);
  }

  // 6. Copy&Swap Buffer Test
  // Draw a Blue box directly to memory using a temporary buffer and copy_buffer
  uint16_t blue_w = 20, blue_h = 20;
  uint32_t blue_size = blue_w * blue_h * sizeof(uint16_t);
  uint16_t *blue_buf = (uint16_t *)sys_malloc(
      SYS_MEM_INTERNAL, blue_size); // Small enough for internal
  if (blue_buf) {
    for (int i = 0; i < blue_w * blue_h; i++)
      blue_buf[i] = 0x001F; // Blue

    // Copy to (400, 400)
    // OffLineSrc = 0 (contiguous)
    // OffLineDst = LCD_WIDTH - blue_w
    lcd_hal_copy_buffer(
        lcd_hal,
        (uint16_t *)draw_buf +
            (400 + LCD_WIDTH *
                       400), // Destination in draw_buf
                             // buffer(此时没有交换内存是不会显示的，但仍可读出)
        blue_buf,           // Source buffer
        blue_w,             // Width
        blue_h,             // Height
        0,                  // OffLineSrc
        LCD_WIDTH - blue_w, // OffLineDst
        0);                 // PixelFormat (assuming 0 is RGB565)

    HAL_Delay(10); // 添加延迟，确保DMA2D传输完成

    read_color = lcd_hal_read_point(lcd_hal, 410, 410);
    if ((uint16_t)read_color != 0x001F) {
      log_e("Copy Buffer Failed: Expected 0x001F, Got 0x%04X", read_color);
    } else {
      log_i("Copy Buffer Test Passed. Point at (410,410) is: 0x%04X",
            read_color);
    }
    sys_free(SYS_MEM_INTERNAL, blue_buf);
  } else {
    log_w("Skipping Copy Buffer Test (Malloc failed)");
  }

  lcd_hal_swap_buffer(lcd_hal);
  HAL_Delay(1000); // 现在应该是可见的(屏幕中间的小蓝块)
  read_color = lcd_hal_read_point(lcd_hal, 410, 410);
  log_i("Post-Swap Read Point at (410,410) is: 0x%04X",
        read_color); // 经过交换后应该读不到颜色

  // 7. Orientation Test
  // Note: changing orientation might affect coordinate system for subsequent
  // tests. We just set it to Landscape (default) to verify the call works.
  lcd_hal_set_orientation(lcd_hal, LCD_ORIENTATION_LANDSCAPE);
  log_i("Set Orientation to LANDSCAPE called");

  // Swap buffers to make sure everything drawn is visible
  lcd_hal_swap_buffer(lcd_hal);
  log_i("Buffer Swapped, tests visual check should be visible now");

  HAL_Delay(1000); // Pause to see test results

  // Bouncing Box Animation Loop - Visual Check
  int16_t x = 0, y = 0;
  int16_t dx = 10, dy = 10;
  uint16_t box_w = 100, box_h = 100;
  uint32_t color = 0xF800; // Red

  // Clear screen for animation start
  lcd_hal_fill_rect(lcd_hal, 0, 0, LCD_WIDTH, LCD_HEIGHT, 0x0000);
  lcd_hal_swap_buffer(lcd_hal);
  lcd_hal_fill_rect(lcd_hal, 0, 0, LCD_WIDTH, LCD_HEIGHT, 0x0000);

  log_i("Starting Animation Loop...");

  while (1) {
    // Clear previous position (simplest is to clear whole screen,
    // optimized is to clear just the previous rect, but let's test fill_rect
    // performance)
    lcd_hal_fill_rect(lcd_hal, 0, 0, LCD_WIDTH, LCD_HEIGHT, 0x0000);

    // Draw Box
    lcd_hal_fill_rect(lcd_hal, x, y, box_w, box_h, color);

    // Update position
    x += dx;
    y += dy;

    // Collision detection
    if (x < 0 || x + box_w >= LCD_WIDTH) {
      dx = -dx;
      x += dx;
      color = (color == 0xF800) ? 0x07E0 : 0xF800; // Toggle Red/Green
    }
    if (y < 0 || y + box_h >= LCD_HEIGHT) {
      dy = -dy;
      y += dy;
      color = (color == 0xF800) ? 0x001F : 0xF800; // Toggle to Blue if red
    }

    // Swap buffers to display
    lcd_hal_swap_buffer(lcd_hal);

    HAL_Delay(50);
  }
}

#endif
