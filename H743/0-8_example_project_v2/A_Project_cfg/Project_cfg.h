/*
 * project_cfg.h
 *
 *  Created on: Feb 5, 2026
 *      Author: 12114
 */

#ifndef APPLICATION_PROJECT_CFG_H_
#define APPLICATION_PROJECT_CFG_H_

#define PRINTF_FLOAT_ENABLED //用于测试程序打印方式选择
//#define RES_BURN_ENABLE	 // 使能系统资源烧录模式

/* ----- 运行配置 ----- */
#define TEST_ENABLE	1		// 使能测试模式(不进入主程序)
#define SERVICE_ENABLE 1	// 服务组件层初始化(底层使能)

#define NETWORK_SERVICE_ENABLE 1	// 网络服务使能(wifi相关)
#define SNTP_SERVICE_ENABLE 1		// 使能SNTP同步RTC
#define CLOUD_SERVICE_ENABLE 1		// 物模型云端控制对接

#define THINGMODEL_ENABLE 1			// 物体模型控制使能

#define LVGL_ENABLE	0		// 图形库使能
#define GUI_ENABLE	0		// UI页面使能

/* ----- log配置 ----- */

/* --- 防止配置项冲突 --- */
#ifdef RES_BURN_ENABLE	// 如果使能烧录模式
#define LVGL_ENABLE	0
#define SERVICE_ENABLE 0
#define THINGMODEL_ENABLE 0
#endif
#if !LVGL_ENABLE		// 未使能图形库
#define GUI_ENABLE 0
#endif
#if !SERVICE_ENABLE		// 未使能服务层
#define NETWORK_SERVICE_ENABLE 0
#define SNTP_SERVICE_ENABLE 0
#define CLOUD_SERVICE_ENABLE 0
#endif
#if !THINGMODEL_ENABLE	// 未使能物模型控制
#define CLOUD_SERVICE_ENABLE 0
#endif

#endif /* APPLICATION_PROJECT_CFG_H_ */
