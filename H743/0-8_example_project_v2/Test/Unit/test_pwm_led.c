#include "test_config.h"
#if ENABLE_TEST_PWM_LED
#include "dev_map.h"
#include "pwm_driver.h"
#include "pwm_factory.h"
#include "pwm_led/pwm_led.h"
#include "test_framework.h"
#include "elog.h"

#define LOG_TAG "TEST_PWM_LED"

static pwm_led_t *pwm_led0;
static pwm_led_t *pwm_led1;

static void test_pwm_led_setup(void) {
  pwm_driver_t *driver0 = pwm_driver_get(PWM_ID_R);
  pwm_driver_t *driver1 = pwm_driver_get(PWM_ID_B);

  if (driver0 == NULL || driver1 == NULL) {
    log_e("PWM drivers not found");
    return;
  }

  // 创建 PWM LED，频率 1kHz，高电平有效 (假设 LED 接法)
  pwm_led0 = pwm_led_create(1000, driver0, 1);
  pwm_led1 = pwm_led_create(1000, driver1, 1);

  log_i("PWM LED Test Setup: PWM0 and PWM1 initialized at 1kHz.");
}

static void test_pwm_led_loop(void) {
  static uint32_t last_tick = 0;
  static int8_t direction = 1;
  static uint8_t brightness = 0;

  // 每 20ms 更新一次亮度，实现呼吸灯效果
  if (sys_get_systick_ms() - last_tick >= 20) {
    last_tick = sys_get_systick_ms();

    pwm_led_set_brightness(pwm_led0, brightness);
    pwm_led_set_brightness(pwm_led1, 100 - brightness); // 交替呼吸

    brightness += direction;
    if (brightness >= 100 || brightness <= 0) {
      direction = -direction;
    }
  }
}

static void test_pwm_led_teardown(void) {
  log_i("PWM LED Test Teardown: Cleaning up.");
  pwm_led_set_brightness(pwm_led0, 0);
  pwm_led_set_brightness(pwm_led1, 0);
  
  pwm_led_destroy(pwm_led0);
  pwm_led_destroy(pwm_led1);
}

REGISTER_TEST(PWM_LED, "PWM LED Breathing Effect on PWM0/PWM1", test_pwm_led_setup, test_pwm_led_loop,
              test_pwm_led_teardown);

#endif
