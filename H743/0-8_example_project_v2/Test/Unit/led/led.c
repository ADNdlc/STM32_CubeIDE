/*
 * led.c
 *
 *  Created on: Feb 5, 2026
 *      Author: 12114
 */

#include "test_framework.h"
#include "test_config.h" // 引入开关配置

// 使用预编译指令包裹整个文件内容
#if ENABLE_TEST_LED

void LED_Setup(void) {
    // 假设 LED 在 PC13
    printf("LED Test Setup...\r\n");
}

void LED_Loop(void) {

    printf("Blink...\r\n");
}

// 注册
REGISTER_TEST(LEDBlink, "Blinks PC13 every 500ms", LED_Setup, LED_Loop, NULL);

#endif // ENABLE_TEST_LED
