/*
 * module_cfg.h
 *
 *  Created on: Dec 29, 2025
 *      Author: 12114
 */

#ifndef PROJECT_CFG_H_
#define PROJECT_CFG_H_

#define CONFIG_RES_BURN_ENABLE 0 // 设置为1时开启烧录模式，烧录完成后应设回0

#define LVGL_ENABLE 1		// 使能lvgl
#define LVGL_DISP_INIT 1	// 屏幕初始化
#define LVGL_INDEV_INIT 1	// 触摸初始化
#define LVGL_FS_INIT 1		// fs接口初始化
#define LVGL_UI_START 0		// 启动UI

#define TEST_ENABLE 1		// 使能测试

#endif /* PROJECT_CFG_H_ */
