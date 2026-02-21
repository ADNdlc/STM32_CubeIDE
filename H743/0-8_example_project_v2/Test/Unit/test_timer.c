#include "test_config.h"
#if ENABLE_TEST_TIMER // 临时开启测试
#include "dev_map.h"
#include "gpio_driver.h"
#include "gpio_factory.h"
#include "test_framework.h"
#include "timer_driver.h"
#include "timer_factory.h"

static timer_driver_t *timer0;
static gpio_driver_t *led0;

#define cycle 1000

static void timer_callback(void *context) {
  if (led0) {
    GPIO_TOGGLE(led0);
    log_i(".");
  }
}

static void test_timer_setup(void) {
  led0 = gpio_driver_get(GPIO_ID_LED0);

  // 使用逻辑 ID 获取定时器驱动
  timer0 = timer_driver_get(TIMER_ID_1);

  if (timer0) {
    TIMER_SET_CALLBACK(timer0, timer_callback, NULL);
    TIMER_SET_PERIOD(timer0, cycle);
    TIMER_START(timer0);
    log_i("Timer Test Setup: Timer ID 1 started with %dms period.", cycle);
  } else {
    log_e("Timer Test Setup: Failed to get timer driver ID 1.");
  }
}

static void test_timer_loop(void) {
  // 逻辑在中断回调中处理
}

static void test_timer_teardown(void) {
  if (timer0) {
    TIMER_STOP(timer0);
    log_i("Timer Test Teardown: Stop Timer ID 1.");
  }
}

REGISTER_TEST(TIMER, "Periodic Timer Test - Toggle LED0 every cycle",
              test_timer_setup, test_timer_loop, test_timer_teardown);

#endif
