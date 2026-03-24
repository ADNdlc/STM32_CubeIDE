#include "test_config.h"
#if ENABLE_TEST_LED
#include "dev_map.h"
#include "gpio_driver.h"
#include "gpio_factory.h"
#include "gpio_led/gpio_led.h"
#include "test_framework.h"

static gpio_led_t *led0;
#ifdef STM32H743xx
static gpio_led_t *led1;
#endif

static void test_led_setup(void) {
  gpio_driver_t *driver0 = gpio_driver_get(GPIO_ID_LED0);
#ifdef STM32H743xx
  gpio_driver_t *driver1 = gpio_driver_get(GPIO_ID_LED1);
#endif
  if (driver0 == NULL) {
    log_e("LED0 not found");
    return;
  }
  led0 = gpio_led_create(driver0, 0);
  gpio_led_set(led0, GPIO_LED_ON);
#ifdef STM32H743xx
  led1 = gpio_led_create(driver1, 0);
  gpio_led_set(led1, GPIO_LED_ON);
#endif

  log_i("LED Test Setup: Ensuring GPIOs are ready.");
}

static void test_led_loop(void) {
  static uint32_t last_tick = 0;
  if (sys_get_systick_ms() - last_tick >= 1000) {
    last_tick = sys_get_systick_ms();
#ifdef STM32H743xx
    gpio_led_toggle(led1);
#endif
    gpio_led_toggle(led0);
    log_d("LED0 Toggled");
  }
}

static void test_led_teardown(void) {
  log_i("LED Test Teardown: Cleaning up.");
  gpio_led_set(led0, GPIO_LED_OFF);
#ifdef STM32H743xx
  gpio_led_set(led1, GPIO_LED_OFF);
#endif
  gpio_led_destroy(led0);
#ifdef STM32H743xx
  gpio_led_destroy(led1);
#endif
}

REGISTER_TEST(LED, "Blink LED0 every 500ms", test_led_setup, test_led_loop,
              test_led_teardown);

#endif
