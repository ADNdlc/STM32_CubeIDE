/*
 * stm32_rtc_driver.h
 *
 *  Created on: Feb 13, 2026
 *      Author: Antigravity
 */

#ifndef PLATFORM_STM32_RTC_DRIVER_H_
#define PLATFORM_STM32_RTC_DRIVER_H_

#include "rtc_driver.h"

#if (TARGET_PLATFORM == PLATFORM_STM32)
#include "rtc.h" // HAL RTC handle hrtc

/**
 * @brief 创建 STM32 RTC 驱动实例
 * @param hrtc HAL RTC 句柄
 * @return 驱动指针
 */
rtc_driver_t *stm32_rtc_driver_create(RTC_HandleTypeDef *hrtc);

#endif

#endif /* PLATFORM_STM32_RTC_DRIVER_H_ */
