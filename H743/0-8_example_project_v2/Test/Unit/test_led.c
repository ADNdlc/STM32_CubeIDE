#include "test_config.h"
#if ENABLE_TEST_LED
#include "gpio.h"
#include "test_framework.h"

static void test_led_setup(void) {
  log_i("LED Test Setup: Ensuring GPIOB is ready.");
}

static void test_led_loop(void) {
  static uint32_t last_tick = 0;
  if (sys_get_systick_ms() - last_tick >= 500) {
    last_tick = sys_get_systick_ms();
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
    log_d("LED0 Toggled");
  }
}

static void test_led_teardown(void) {
  log_i("LED Test Teardown: Cleaning up.");
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
}

REGISTER_TEST(LED, "Blink LED0 every 500ms", test_led_setup, test_led_loop,
              test_led_teardown);

#endif
