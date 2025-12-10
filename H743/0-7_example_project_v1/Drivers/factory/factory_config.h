/*
 * factory_config.h
 *
 *  Created on: Dec 10, 2025
 *      Author: 12114
 */

#ifndef FACTORY_FACTORY_CONFIG_H_
#define FACTORY_FACTORY_CONFIG_H_

/* 
 * 平台选择宏定义
 * 用于决定各工厂返回哪个平台的驱动实现
 */
#define PLATFORM_STM32 1
//#define PLATFORM_XXX   2

// 默认使用STM32平台
#ifndef TARGET_PLATFORM
#define TARGET_PLATFORM PLATFORM_STM32
#endif

/* 各个驱动模块的平台选择配置 */
#if (TARGET_PLATFORM == PLATFORM_STM32)
  #define GPIO_DRIVER_PLATFORM      PLATFORM_STM32
  #define PWM_DRIVER_PLATFORM       PLATFORM_STM32
  #define USART_DRIVER_PLATFORM     PLATFORM_STM32
  #define LCD_DRIVER_PLATFORM       PLATFORM_STM32
  #define SDRAM_DRIVER_PLATFORM     PLATFORM_STM32
#else
  #error "未定义有效的目标平台，请检查TARGET_PLATFORM配置"
#endif

#endif /* FACTORY_FACTORY_CONFIG_H_ */
