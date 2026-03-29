#include "test_config.h"
#if ENABLE_TEST_RGB_LED
#include "dev_map.h"
#include "pwm_driver.h"
#include "pwm_factory.h"
#include "pwm_led/pwm_led.h"
#include "rgb_led/rgb_led.h"
#include "test_framework.h"

#define LOG_TAG "TEST_RGB_LED"
#include "elog.h"

static rgb_led_t *rgb_led;
static pwm_led_t *led_r;
static pwm_led_t *led_g;
static pwm_led_t *led_b;

static void test_rgb_led_setup(void) {
  pwm_driver_t *dr = pwm_driver_get(PWM_ID_R);
  pwm_driver_t *dg = pwm_driver_get(PWM_ID_G);
  pwm_driver_t *db = pwm_driver_get(PWM_ID_B);

  if (!dr || !dg || !db) {
    log_e("PWM drivers for RGB not found");
    return;
  }

  // 创建底层的 PWM LED (1kHz, 高电平有效)
  led_r = pwm_led_create(1000, dr, 1);
  led_g = pwm_led_create(1000, dg, 1);
  led_b = pwm_led_create(1000, db, 1);

  // 创建 RGB LED 组合对象
  rgb_led = rgb_led_create(led_r, led_g, led_b);

  log_i("RGB LED Test Setup: R, G, B PWMs and RGB component initialized.");
}

static void test_rgb_led_loop(void) {
  static uint32_t last_tick = 0;
  static uint8_t stage = 0;
  static uint16_t hue = 0;

  uint32_t now = sys_get_systick_ms();

  // 每 1000ms 切换一次固定的 RGB 颜色
  if (stage < 8) {
    if (now - last_tick >= 1000) {
      last_tick = now;
      uint32_t colors[] = {
          0xFF0000, // Red
          0x00FF00, // Green
          0x0000FF, // Blue
          0x00FFFF, // Cyan
          0xFF00FF, // Magenta
          0xFFFF00, // Yellow
          0xFFFFFF, // White
          0x000000  // Off
      };
      log_d("Setting RGB Stage %d: 0x%06X", stage, colors[stage]);
      rgb_led_set_color(rgb_led, colors[stage]);
      stage++;
    }
  } 
  // 之后进入 HSV 彩虹循环
  else {
    if (now - last_tick >= 20) {
      last_tick = now;
      rgb_led_set_hsv(rgb_led, hue, 100, 50); // 50% 亮度彩虹
      hue = (hue + 1) % 360;
      if (hue == 0) {
        log_d("RGB HSV Cycle Completed");
      }
    }
  }
}

static void test_rgb_led_teardown(void) {
  log_i("RGB LED Test Teardown: Cleaning up.");
  rgb_led_set_color(rgb_led, 0);
  
  // 销毁顺序：先销毁组合对象，再销毁底层对象（或者由组合对象负责销毁底层，这里手动销毁驱动更明确）
  rgb_led_destroy(rgb_led);
  pwm_led_destroy(led_r);
  pwm_led_destroy(led_g);
  pwm_led_destroy(led_b);
}

REGISTER_TEST(RGB_LED, "RGB LED standard colors and HSV rainbow", test_rgb_led_setup, test_rgb_led_loop,
              test_rgb_led_teardown);

#endif
