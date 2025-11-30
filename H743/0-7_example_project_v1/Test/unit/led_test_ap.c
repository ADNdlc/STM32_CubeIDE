/*
 * led_test_app.c
 *
 *  Created on: Nov 30, 2025
 *      Author: 12114
 */

#include "led_test_app.h"
#include "device_mapping.h"
#include "gpio_factory.h"
#include "gpio_led/gpio_led.h"
#include "main.h" // For HAL_Delay
#include "pwm_factory.h"
#include "pwm_led/pwm_led.h"
#include "rgb_led/rgb_led.h"


// 延时辅助函数
static void delay_ms(uint32_t ms) { HAL_Delay(ms); }

// 1. GPIO LED 测试用例
static void test_gpio_led(void) {
  // 获取驱动实例 (使用 GPIO_LED_RED 作为测试对象)
  gpio_driver_t *driver = gpio_driver_get(GPIO_LED_RED);

  if (driver) {
    // 创建 GPIO LED 对象 (高电平有效)
    gpio_led_t *led = gpio_led_create(driver, 1);

    if (led) {
      // 测试开启
      led_hal_on((led_hal_t *)led);
      delay_ms(500);

      // 测试关闭
      led_hal_off((led_hal_t *)led);
      delay_ms(500);

      // 销毁对象
      gpio_led_delete(led);
    }
  }
}

// 2. PWM LED 测试用例
static void test_pwm_led(void) {
  // 获取驱动实例 (使用 PWM_LED_GREEN 作为测试对象)
  pwm_driver_t *driver = pwm_driver_get(PWM_LED_GREEN);

  if (driver) {
    // 创建 PWM LED 对象 (1kHz)
    pwm_led_t *led = pwm_led_create(1000, driver);

    if (led) {
      // 测试开启 (全亮)
      led_hal_on((led_hal_t *)led);
      delay_ms(500);

      // 测试调节亮度 (50%)
      pwm_led_set_brightness(led, 500);
      delay_ms(500);

      // 测试关闭
      led_hal_off((led_hal_t *)led);
      delay_ms(500);

      // 销毁对象
      pwm_led_delete(led);
    }
  }
}

// 3. RGB LED 测试用例
static void test_rgb_led(void) {
  // 获取三个通道的驱动
  pwm_driver_t *r_drv = pwm_driver_get(PWM_LED_RED);
  pwm_driver_t *g_drv = pwm_driver_get(PWM_LED_GREEN);
  pwm_driver_t *b_drv = pwm_driver_get(PWM_LED_BLUE);

  if (r_drv && g_drv && b_drv) {
    // 创建三个 PWM LED 对象
    pwm_led_t *r_led = pwm_led_create(1000, r_drv);
    pwm_led_t *g_led = pwm_led_create(1000, g_drv);
    pwm_led_t *b_led = pwm_led_create(1000, b_drv);

    if (r_led && g_led && b_led) {
      // 创建 RGB LED 对象 (聚合三个 PWM LED)
      rgb_led_t *rgb = rgb_led_create(r_led, g_led, b_led);

      if (rgb) {
        // 测试红色
        rgb_led_set_color(rgb, 0xFF0000);
        led_hal_on((led_hal_t *)rgb);
        delay_ms(500);

        // 测试绿色
        rgb_led_set_color(rgb, 0x00FF00);
        led_hal_on((led_hal_t *)rgb);
        delay_ms(500);

        // 测试蓝色
        rgb_led_set_color(rgb, 0x0000FF);
        led_hal_on((led_hal_t *)rgb);
        delay_ms(500);

        // 测试关闭
        led_hal_off((led_hal_t *)rgb);
        delay_ms(500);

        // 销毁 RGB LED
        rgb_led_delete(rgb);
      }

      // 销毁 PWM LED (RGB LED 不负责销毁它们)
      pwm_led_delete(r_led);
      pwm_led_delete(g_led);
      pwm_led_delete(b_led);
    }
  }
}

// 4. 多态行为测试用例
static void test_polymorphism(void) {
  // 准备资源
  gpio_driver_t *gpio_drv = gpio_driver_get(GPIO_LED_BLUE);
  pwm_driver_t *pwm_drv = pwm_driver_get(PWM_LED_RED);

  if (gpio_drv && pwm_drv) {
    gpio_led_t *led1 = gpio_led_create(gpio_drv, 1);
    pwm_led_t *led2 = pwm_led_create(1000, pwm_drv);

    if (led1 && led2) {
      // 定义多态数组
      led_hal_t *leds[] = {(led_hal_t *)led1, (led_hal_t *)led2};
      int led_count = sizeof(leds) / sizeof(leds[0]);

      // 统一操作
      for (int i = 0; i < led_count; i++) {
        // 统一开启
        led_hal_on(leds[i]);
        delay_ms(200);

        // 统一关闭
        led_hal_off(leds[i]);
        delay_ms(200);
      }

      // 销毁
      gpio_led_delete(led1);
      pwm_led_delete(led2);
    }
  }
}

// 主测试入口
void led_test_run(void) {
  test_gpio_led();
  test_pwm_led();
  test_rgb_led();
  test_polymorphism();
}
