/*
 * project_cfg.h
 *
 *  Created on: Feb 5, 2026
 *      Author: 12114
 */

#ifndef APPLICATION_PROJECT_CFG_H_
#define APPLICATION_PROJECT_CFG_H_

/*
 * 平台宏定义
 * 用于决定使用哪个平台的dev_map映射，以及各工厂返回哪个平台的驱动实现。
 */
#define PLATFORM_STM32 1
// #define PLATFORM_XXX   2

/*
 * 硬件版本定义
 * 用于决定使用哪个版本的dev_map映射。
 */
#define STM32H743_BOARD_V1 1


#define TARGET_PLATFORM PLATFORM_STM32	// 平台选择


#endif /* APPLICATION_PROJECT_CFG_H_ */
