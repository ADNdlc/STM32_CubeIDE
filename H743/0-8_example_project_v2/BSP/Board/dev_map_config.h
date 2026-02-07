/*
 * dev_map_config.h
 *
 *  Created on: Feb 7, 2026
 *      Author: 12114
 */

#ifndef BOARD_DEV_MAP_CONFIG_H_
#define BOARD_DEV_MAP_CONFIG_H_

#include "../../A_Project_cfg/Project_cfg.h"

// 默认使用STM32平台
#ifndef TARGET_PLATFORM
#define TARGET_PLATFORM PLATFORM_STM32
#endif

/* 平台选择 */
#if (TARGET_PLATFORM == PLATFORM_STM32)
/*
 * 硬件版本定义
 * 用于决定使用哪个版本的dev_map映射。
 */
#define STM32H743_BOARD_V1 1

/* 硬件版本选择 */
#define TARGET_BOARD STM32H743_BOARD_V1
#else
#error "dev_map_config: 未定义有效的目标平台，请检查TARGET_PLATFORM配置"
#endif

#endif /* BOARD_DEV_MAP_CONFIG_H_ */
