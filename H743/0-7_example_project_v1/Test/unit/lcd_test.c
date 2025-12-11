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
#define LCD_WIDTH 800
#define LCD_HEIGHT 480
#define BUFFER_SIZE (LCD_WIDTH * LCD_HEIGHT * 2)

void lcd_test_run(void) {
  log_i("LCD Test Start");
  lcd_driver_t *lcd_driver = lcd_driver_get(LCD_MAIN);
  if (lcd_driver == NULL) {
    log_e("LCD Driver get failed");
    return;
  }

  // Allocate buffers in external memory (SDRAM)
  uint16_t *display_buf = (uint16_t *)sys_malloc(SYS_MEM_EXTERNAL, BUFFER_SIZE);
  uint16_t *draw_buf = (uint16_t *)sys_malloc(SYS_MEM_EXTERNAL, BUFFER_SIZE);

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

  log_e("LCD HAL create Success");

  // Bouncing Box Animation
  int16_t x = 0, y = 0;
  int16_t dx = 10, dy = 10;
  uint16_t box_w = 100, box_h = 100;
  uint32_t color = 0xF800; // Red

  // Fill screen with black initially
  lcd_hal_fill_rect(lcd_hal, 0, 0, LCD_WIDTH, LCD_HEIGHT, 0x0000);
  lcd_hal_swap_buffer(lcd_hal);
  lcd_hal_fill_rect(lcd_hal, 0, 0, LCD_WIDTH, LCD_HEIGHT, 0x0000);

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
