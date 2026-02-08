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


#define TARGET_PLATFORM PLATFORM_STM32	// 平台选择

/* ----- 运行配置 ----- */
#define TEST_ENABLE	1	// 使能测试模式(不进入主程序)

#endif /* APPLICATION_PROJECT_CFG_H_ */
