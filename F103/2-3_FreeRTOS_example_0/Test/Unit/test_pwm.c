#include "test_config.h"
#if ENABLE_TEST_PWM
#include "dev_map.h"
#include "test_framework.h"

static pwm_driver_t *pwm_r = NULL;
static uint32_t duty = 0;
static int8_t direction = 1;
static uint32_t last_tick = 0;

static void test_pwm_setup(void) {
  pwm_r = pwm_driver_get(PWM_ID_R);
  if (pwm_r == NULL) {
    log_e("PWM_ID_R not found");
    return;
  }

  // 测试 set_freq 功能，设置为 1kHz
  PWM_SET_FREQ(pwm_r, 1000);

  // 测试 get_freq 功能并打印
  uint32_t freq = PWM_GET_FREQ(pwm_r);
  log_i("PWM Test Setup: PWM_ID_R initialized, Actual Freq: %lu Hz", freq);

  // 测试 start 功能
  PWM_START(pwm_r);

  last_tick = sys_get_systick_ms();
}

static void test_pwm_loop(void) {
  if (pwm_r == NULL) return;

  uint32_t current_tick = sys_get_systick_ms();

  // 每 10ms 更新一次占空比，实现呼吸灯效果
  if (current_tick - last_tick >= 10) {
    last_tick = current_tick;

    // 测试 get_duty_max 功能
    uint32_t max_duty = PWM_GET_DUTY_MAX(pwm_r);
    uint32_t step = max_duty / 50; // 2% 步进
    if (step == 0)
      step = 1;

    if (direction > 0) {
      duty += step;
      if (duty >= max_duty) {
        duty = max_duty;
        direction = -1;
      }
    } else {
      if (duty <= step) {
        duty = 0;
        direction = 1;
      } else {
        duty -= step;
      }
    }

    // 测试 set_duty 功能
    PWM_SET_DUTY(pwm_r, duty);
  }
}

static void test_pwm_teardown(void) {
  if (pwm_r) {
    // 测试 stop 功能
    PWM_STOP(pwm_r);
  }
  log_i("PWM Test Teardown: PWM stopped.");
}

REGISTER_TEST(PWM, "PWM Breathing Light on PWM_ID_R", test_pwm_setup,
              test_pwm_loop, test_pwm_teardown);

#endif
