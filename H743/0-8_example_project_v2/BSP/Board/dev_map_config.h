/*
 * dev_map_config.h
 *
 *  Created on: Feb 7, 2026
 *      Author: 12114
 */

#ifndef BOARD_DEV_MAP_CONFIG_H_
#define BOARD_DEV_MAP_CONFIG_H_

#include "Project_cfg.h"

// 默认使用STM32平台
#ifndef TARGET_PLATFORM
#define TARGET_PLATFORM PLATFORM_STM32
#endif

/* 硬件版本选择 */
#if (TARGET_PLATFORM == PLATFORM_STM32)
#define TARGET_BOARD STM32H743_BOARD_V1
#else
#error "dev_map_config: 未定义有效的目标平台，请检查TARGET_PLATFORM配置"
#endif

#endif /* BOARD_DEV_MAP_CONFIG_H_ */
