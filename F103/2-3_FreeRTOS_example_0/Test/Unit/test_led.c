#include "test_config.h"
#if ENABLE_TEST_LED
#include "dev_map.h"
#include "test_framework.h"

static gpio_driver_t *led0;
#ifdef STM32H743xx
static gpio_driver_t *led1;
#endif

// 添加静态变量来跟踪上次翻转时间
static uint32_t led_last_toggle_time = 0;

void test_led_on(void) { GPIO_WRITE(led0, 0); }

void test_led_off(void) { GPIO_WRITE(led0, 1); }

#ifdef STM32H743xx
void test_led2_on(void) { GPIO_WRITE(led1, 1); }
void test_led2_off(void) { GPIO_WRITE(led1, 0); }
#endif

static void test_led_setup(void) {
  led0 = gpio_driver_get(GPIO_ID_LED0);
#ifdef STM32H743xx
  led1 = gpio_driver_get(GPIO_ID_LED1);
#endif
  if (led0 == NULL) {
    log_e("LED0 not found");
    return;
  }

  // 显式初始化 GPIO 模式
  GPIO_SET_MODE(led0, GPIO_PushPullOutput);

#ifdef STM32H743xx
  if (led1 == NULL) {
    log_e("LED1 not found");
    return;
  }
  GPIO_SET_MODE(led1, GPIO_PushPullOutput);
#endif
  
  // 初始化上次翻转时间为当前时间
  led_last_toggle_time = sys_get_systick_ms();
  
  log_i("LED Test Setup: Ensuring GPIOs are ready.");
}

static void test_led_loop(void) {
  uint32_t current_time = sys_get_systick_ms();
	log_d("LED0 loop at %lu ms", current_time);
  if ((current_time - led_last_toggle_time) >= 1000) {
    led_last_toggle_time = current_time;
#ifdef STM32H743xx
    GPIO_TOGGLE(led1);
#endif
    GPIO_TOGGLE(led0);
	log_d("LED0 Toggled at %lu ms", current_time);
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
