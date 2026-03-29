/*
 * project_cfg.h
 *
 *  Created on: Feb 5, 2026
 *      Author: 12114
 */

#ifndef APPLICATION_PROJECT_CFG_H_
#define APPLICATION_PROJECT_CFG_H_

#if defined(_WIN32) || defined(__cplusplus)
#define USE_Simulator 1 //  模拟器环境
#endif

#if USE_Simulator
#pragma message("Environment: LVGL Simulator (VS/Windows)")
#else
#pragma message("Environment: STM32 Hardware")
#endif

#define PRINTF_FLOAT_ENABLED

/* ======================================================= */
/* 1. 选择系统当前的大运行模式 (解开一个即可)         */
/* ======================================================= */

//#define SYS_PROFILE_RELEASE      // 正常发布模式 (默认)
//#define SYS_PROFILE_BURN_RES // 资源烧录与验证模式
#define SYS_PROFILE_UNIT_TEST    // 底层单元测试模式

/* ======================================================= */
/* 2. 根据选定的 Profile，自动映射底层功能组件开关               */
/*    (绝对不再使用 #undef，直接在这里进行逻辑分支)               */
/* ======================================================= */

#if defined(SYS_PROFILE_BURN_RES)
// --- 烧录模式配置 ---
#define RES_BURN_ENABLE 1    // 开启烧录逻辑
#define RES_DISPLAY_ENABLE 1 // 烧录后死循环显示测试
#define RES_USE_INTERNAL 1   // 必须把图片编译进固件用于烧录
#define LVGL_ENABLE 1        // 需要 LVGL 验证显示

// 强制关闭无关业务组件，腾出 RAM 和 CPU
#define GUI_ENABLE 0
#define SERVICE_ENABLE 0
#define NETWORK_SERVICE_ENABLE 0
#define SNTP_SERVICE_ENABLE 0
#define CLOUD_SERVICE_ENABLE 0
#define THINGMODEL_ENABLE 0
#define TEST_ENABLE 0

#elif defined(SYS_PROFILE_UNIT_TEST)
// --- 单元测试模式配置 ---
#define TEST_ENABLE 1
#define RES_USE_INTERNAL 0 // 测试不用带大图片
#define RES_BURN_ENABLE 0
#define RES_DISPLAY_ENABLE 0
#define LVGL_ENABLE 0
#define GUI_ENABLE 0
#define SERVICE_ENABLE 0
// ... 其他全关

#else
// --- 正常发布/运行模式配置 (SYS_PROFILE_RELEASE) ---
#define RES_BURN_ENABLE 0
#define RES_DISPLAY_ENABLE 0
// 运行态是否使用片上flash存储资源
#define RES_USE_INTERNAL 0

// 开启业务系统
#define LVGL_ENABLE 1
#define GUI_ENABLE 1
#define SERVICE_ENABLE 1
#define NETWORK_SERVICE_ENABLE 1
#define SNTP_SERVICE_ENABLE 1
#define THINGMODEL_ENABLE 1
#define CLOUD_SERVICE_ENABLE 1
#define TEST_ENABLE 0
#endif
#endif /* APPLICATION_PROJECT_CFG_H_ */
