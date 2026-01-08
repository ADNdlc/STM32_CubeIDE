/*
 * module_cfg.h
 *
 *  Created on: Dec 29, 2025
 *      Author: 12114
 */

#ifndef PROJECT_CFG_H_
#define PROJECT_CFG_H_

#define CONFIG_RES_BURN_ENABLE 0 // 设置为1时开启烧录模式，烧录完成后应设回0

#if defined(_WIN32) || defined(__cplusplus)
#define USE_Simulator 1
#endif
 // Compile-time check
#if USE_Simulator
#pragma message("Environment: LVGL Simulator (VS/Windows)")
#else
#pragma message("Environment: STM32 Hardware")
#endif

#define LVGL_ENABLE 1
#define LVGL_DISP_INIT 1
#define LVGL_INDEV_INIT 1
#define LVGL_FS_INIT 1

#define SYS_FLASH_HANDLER_ENABLE 1
#define SYS_CONFIG_ENABLE 1
#define NET_MGR_ENABLE 1
#define SYS_STATE_ENABLE 1
#define THING_MODEL_ENABLE 1

#define TEST_ENABLE 0

#endif /* PROJECT_CFG_H_ */
