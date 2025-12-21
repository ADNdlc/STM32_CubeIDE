/*
 * stm32_rtc_driver.h
 *
 *  Created on: Dec 21, 2025
 *      Author: 12114
 */

#ifndef DRIVERS_PLATFORM_STM32_DEVICE_STM32_RTC_DRIVER_H_
#define DRIVERS_PLATFORM_STM32_DEVICE_STM32_RTC_DRIVER_H_

#include "rtc_driver.h"
#include "stm32h7xx_hal.h"

// STM32 RTC 驱动结构体
typedef struct {
    rtc_driver_t base;  // 继承自 rtc_driver_t 基类
    RTC_HandleTypeDef *hrtc;  // STM32特有的RTC句柄
} stm32_rtc_driver_t;

// 创建 STM32 RTC 驱动实例
rtc_driver_t* stm32_rtc_driver_create(RTC_HandleTypeDef *hrtc);

#endif /* DRIVERS_PLATFORM_STM32_DEVICE_STM32_RTC_DRIVER_H_ */