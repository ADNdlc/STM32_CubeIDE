/*
 * led_test_app.c
 *
 *  Created on: Nov 30, 2025
 *      Author: 12114
 */
#include "all_tests_config.h"
#if led_test

#include "device_mapping.h"
#include "gpio_factory.h"
#include "gpio_led/gpio_led.h"
#include "led_test.h"
#include "main.h"
#include "pwm_factory.h"
#include "pwm_led/pwm_led.h"
#include "rgb_led/rgb_led.h"

// 延时辅助函数
static void delay_ms(uint32_t ms) { HAL_Delay(ms); }

// 1. GPIO LED 测试用例
static void test_gpio_led(void) {
  // 获取驱动实例 (使用 GPIO_LED_RED 作为测试对象)
  gpio_driver_t *driver = gpio_driver_get(GPIO_LED_0);

  if (driver) {
    // 创建 GPIO LED 对象 (高电平有效)
    gpio_led_t *led = gpio_led_create(driver, 0);

    if (led) {
      int i = 3;
      while (i--) {
        led_hal_on((led_hal_t *)led);
        delay_ms(200);

        led_hal_off((led_hal_t *)led);
        delay_ms(200);
      }

      // 销毁对象
      gpio_led_delete(led);
    }
  }
}

// 2. PWM LED 测试用例
static void test_pwm_led(void) {
  // 获取驱动实例 (使用 PWM_LED_GREEN 作为测试对象)
  pwm_driver_t *driver = pwm_driver_get(PWM_LED_1);

  if (driver) {
    // 创建 PWM LED 对象 (1kHz)
    pwm_led_t *led = pwm_led_create(1000, 0, driver);

    if (led) {
      // 开启 (全亮)
      led_hal_on((led_hal_t *)led);
      delay_ms(500);

      // 测试调节亮度 (50%)
      pwm_led_set_brightness(led, 500);
      delay_ms(500);

      // 关闭
      led_hal_off((led_hal_t *)led);
      delay_ms(500);

      led_hal_on((led_hal_t *)led);
      // 呼吸
      for (int i = 0; i < 100; i++) {
        pwm_led_set_brightness(led, i * 10);
        delay_ms(25);
      }
      for (int i = 100; i > 0; i--) {
        pwm_led_set_brightness(led, i * 10);
        delay_ms(25);
      }
      led_hal_off((led_hal_t *)led);

      // 销毁对象
      pwm_led_delete(led);
    }
  }
}

// 3. RGB LED 测试用例
static void test_rgb_led(void) {
  // 获取三个通道的驱动
  pwm_driver_t *r_drv = pwm_driver_get(RGB_LED_RED);
  pwm_driver_t *g_drv = pwm_driver_get(RGB_LED_GREEN);
  pwm_driver_t *b_drv = pwm_driver_get(RGB_LED_BLUE);

  if (r_drv && g_drv && b_drv) {
    // 创建三个 PWM LED 对象
    pwm_led_t *r_led = pwm_led_create(1000, 1, r_drv);
    pwm_led_t *g_led = pwm_led_create(1000, 1, g_drv);
    pwm_led_t *b_led = pwm_led_create(1000, 1, b_drv);

    if (r_led && g_led && b_led) {
      // 创建 RGB LED 对象 (聚合三个 PWM LED)
      rgb_led_t *rgb = rgb_led_create(r_led, g_led, b_led);

      if (rgb) {
        // 先打开RGB LED
        led_hal_on((led_hal_t *)rgb);

        // 测试纯红色 (HSV: H=0, S=100, V=100)
        rgb_led_set_hsv(rgb, 0, 100, 100);
        delay_ms(500);

        // 测试纯绿色 (HSV: H=120, S=100, V=100)
        rgb_led_set_hsv(rgb, 120, 100, 100);
        delay_ms(500);

        // 测试纯蓝色 (HSV: H=240, S=100, V=100)
        rgb_led_set_hsv(rgb, 240, 100, 100);
        delay_ms(500);

        // 测试黄色 (HSV: H=60, S=100, V=100)
        rgb_led_set_hsv(rgb, 60, 100, 100);
        delay_ms(500);

        // 测试青色 (HSV: H=180, S=100, V=100)
        rgb_led_set_hsv(rgb, 180, 100, 100);
        delay_ms(500);

        // 测试品红色 (HSV: H=300, S=100, V=100)
        rgb_led_set_hsv(rgb, 300, 100, 100);
        delay_ms(500);

        // 测试关闭
        led_hal_off((led_hal_t *)rgb);
        delay_ms(500);

        // 重新打开
        led_hal_on((led_hal_t *)rgb);

        // 彩虹色循环 - 固定饱和度和亮度，只改变色相
        // 这样颜色变化会非常明显
        for (int h = 0; h < 360; h += 3) {
          rgb_led_set_hsv(rgb, h, 100, 100); // 满饱和度，满亮度
          delay_ms(20);
        }

        delay_ms(500);

        // 亮度渐变测试 - 固定红色，改变亮度
        for (int v = 0; v <= 100; v += 2) {
          rgb_led_set_hsv(rgb, 0, 100, v); // 红色，满饱和度，亮度渐变
          delay_ms(50);
        }
        for (int v = 100; v >= 0; v -= 2) {
          rgb_led_set_hsv(rgb, 0, 100, v);
          delay_ms(50);
        }

        led_hal_off((led_hal_t *)rgb);

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
  gpio_driver_t *gpio_drv = gpio_driver_get(GPIO_LED_0);
  pwm_driver_t *pwm_drv = pwm_driver_get(PWM_LED_1);
  
  // 获取RGB LED所需的PWM驱动
  pwm_driver_t *r_drv = pwm_driver_get(RGB_LED_RED);
  pwm_driver_t *g_drv = pwm_driver_get(RGB_LED_GREEN);
  pwm_driver_t *b_drv = pwm_driver_get(RGB_LED_BLUE);

  if (gpio_drv && pwm_drv && r_drv && g_drv && b_drv) {
    gpio_led_t *led1 = gpio_led_create(gpio_drv, 0);
    pwm_led_t *led2 = pwm_led_create(1000, 0, pwm_drv);

    // 创建RGB LED组件
    pwm_led_t *r_led = pwm_led_create(1000, 1, r_drv);
    pwm_led_t *g_led = pwm_led_create(1000, 1, g_drv);
    pwm_led_t *b_led = pwm_led_create(1000, 1, b_drv);
    
    if (r_led && g_led && b_led) {
      rgb_led_t *rgb_led = rgb_led_create(r_led, g_led, b_led);

      if (led1 && led2 && rgb_led) {
        // 定义多态数组
        led_hal_t *leds[] = {(led_hal_t *)led1, (led_hal_t *)led2, (led_hal_t *)rgb_led};
        int led_count = sizeof(leds) / sizeof(leds[0]);

        // 使用led_hal_set_data统一初始赋值
        for (int i = 0; i < led_count; i++) {
            led_hal_set_data(leds[i], 0x00FFFFFF);
        }

        // 同步开启关闭几次测试
        for (int j = 0; j < 5; j++) {
          // 统一开启
          for (int i = 0; i < led_count; i++) {
            led_hal_on(leds[i]);
          }
          delay_ms(500);

          // 统一关闭
          for (int i = 0; i < led_count; i++) {
            led_hal_off(leds[i]);
          }
          delay_ms(500);
        }

        // 销毁
        gpio_led_delete(led1);
        pwm_led_delete(led2);
        rgb_led_delete(rgb_led);
      }
      
      // 销毁RGB LED组件（如果rgb_led_create失败）
      if (r_led) pwm_led_delete(r_led);
      if (g_led) pwm_led_delete(g_led);
      if (b_led) pwm_led_delete(b_led);
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

#endif
