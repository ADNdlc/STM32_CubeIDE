/*
 * test_pwm_led.c
 *
 *  Created on: Nov 27, 2025
 *      Author: 12114
 */

#include "pwm_led.h"
#include "stm32_pwm_adapter.h"
#include <stddef.h>

// Mock GPIO ops
void mock_write_pin(void *port, uint16_t pin, uint8_t value) {}
uint8_t mock_read_pin(void *port, uint16_t pin) { return 0; }
void mock_toggle_pin(void *port, uint16_t pin) {}

led_gpio_ops_t mock_gpio_ops = {
    .write_pin = mock_write_pin,
    .read_pin = mock_read_pin,
    .toggle_pin = mock_toggle_pin,
};

// Mock TIM handle
typedef struct {
  void *Instance;
}
TIM_HandleTypeDef htim1;

int main(void) {
  // 1. 创建 STM32 PWM 驱动适配器
  // 注意：这里需要强制转换 mock 的 handle，实际代码中不需要
  stm32_pwm_driver_t *stm32_driver = stm32_pwm_driver_create((void *)&htim1, 1);

  // 2. 创建 PWM LED，注入驱动
  // 假设 port 为 NULL，pin 为 0，因为 PWM 模式下可能不直接操作 GPIO，或者由 HAL
  // 库接管
  pwm_led_t *my_pwm_led =
      pwm_led_create(NULL, 0, &mock_gpio_ops, (pwm_driver_t *)stm32_driver);

  // 3. 使用多态接口
  led_on((led_t *)my_pwm_led);

  // 4. 使用子类特有接口
  pwm_led_set_brightness(my_pwm_led, 500);

  led_off((led_t *)my_pwm_led);

  return 0;
}
