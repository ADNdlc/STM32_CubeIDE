#include "test_config.h"
#if ENABLE_TEST_LED
#include "dev_map.h"
#include "gpio_driver.h"
#include "gpio_factory.h"
#include "test_framework.h"

static gpio_driver_t *led0;
#ifdef STM32H743xx
static gpio_driver_t *led1;
#endif

void test_led_on(void) { GPIO_WRITE(led0, 1); }

void test_led_off(void) { GPIO_WRITE(led0, 0); }

#ifdef STM32H743xx
void test_led2_on(void) { GPIO_WRITE(led1, 1); }
void test_led2_off(void) { GPIO_WRITE(led1, 0); }
#endif

static void test_led_setup(void) {
  led0 = gpio_driver_get(GPIO_ID_LED0);
#ifdef STM32H743xx
  led1 = gpio_driver_get(GPIO_ID_LED1);
#endif
  log_i("LED Test Setup: Ensuring GPIOB is ready.");
}

static void test_led_loop(void) {
  static uint32_t last_tick = 0;
  if (sys_get_systick_ms() - last_tick >= 500) {
    last_tick = sys_get_systick_ms();
#ifdef STM32H743xx
    GPIO_TOGGLE(led1);
#endif
    GPIO_TOGGLE(led0);
    log_d("LED0 Toggled");
  }
}

static void test_led_teardown(void) {
  log_i("LED Test Teardown: Cleaning up.");
#ifdef STM32H743xx
  GPIO_WRITE(led1, 1);
#endif
  GPIO_WRITE(led0, 1);
}

REGISTER_TEST(LED, "Blink LED0 every 500ms", test_led_setup, test_led_loop,
              test_led_teardown);

#endif
