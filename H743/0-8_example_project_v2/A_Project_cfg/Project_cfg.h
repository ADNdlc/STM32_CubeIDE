/*
 * project_cfg.h
 *
 *  Created on: Feb 5, 2026
 *      Author: 12114
 */

#ifndef APPLICATION_PROJECT_CFG_H_
#define APPLICATION_PROJECT_CFG_H_

#define PRINTF_FLOAT_ENABLED //用于测试程序打印方式
//#define RES_BURN_ENABLE		 // 使能系统资源烧录模式

/* ----- 运行配置 ----- */
#define TEST_ENABLE	1	// 使能测试模式(不进入主程序)
#define SERVICE_ENABLE 0// 服务组件层
#define LVGL_ENABLE	1	// 图形库使能
#define GUI_ENABLE	0	// UI页面使能

#ifdef RES_BURN_ENABLE
#define GUI_ENABLE	0
#endif
#endif /* APPLICATION_PROJECT_CFG_H_ */
