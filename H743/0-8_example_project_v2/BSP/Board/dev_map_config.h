/*
 * dev_map_config.h
 *
 *  Created on: Feb 7, 2026
 *      Author: 12114
 */

#ifndef BOARD_DEV_MAP_CONFIG_H_
#define BOARD_DEV_MAP_CONFIG_H_

/******************************************************
 * 平台宏定义
 * 用于决定使用哪个平台的dev_map映射，以及各总线工厂返回哪个平台的驱动实现。
 */
#define PLATFORM_STM32 1
// #define PLATFORM_XXX   2

/* 平台选择 */
#define TARGET_PLATFORM PLATFORM_STM32 // 平台选择

/******************************************************
 * 硬件版本定义
 * 用于决定使用哪个版本的dev_map映射。
 */
#if (TARGET_PLATFORM == PLATFORM_STM32)
#define STM32H743_BOARD_V1 1
// #define XXX_BOARD_VXX  2

#define TARGET_BOARD STM32H743_BOARD_V1 // 硬件版本选择
#else
#error "dev_map_config: 未定义有效的目标平台，请检查TARGET_PLATFORM配置"
#endif

/******************************************************
 * 各个硬件类型选择
 * 用于决定使用哪个版本的设备驱动创建
 */
/* 温湿度传感器类型 */
#define USE_DHT11 1
// #define USE_DHT22 2
#define DEV_HUMITURE USE_DHT11 // 温湿度传感器选择

/* 光照传感器类型 */
#define USE_BH1750 1
//...
#define DEV_LIGHT USE_BH1750 // 光照传感器选择

#endif /* BOARD_DEV_MAP_CONFIG_H_ */
