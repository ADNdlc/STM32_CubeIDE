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
#define USE_Simulator 1 //  模拟器环境
#endif

#if USE_Simulator
#pragma message("Environment: LVGL Simulator (VS/Windows)")
#else
#pragma message("Environment: STM32 Hardware")
#include "device_mapping.h"
#endif

#define TEST_ENABLE 1

#define LVGL_ENABLE 0
#define LVGL_DISP_INIT 1
#define LVGL_INDEV_INIT 1
#define LVGL_FS_INIT 0

#if !TEST_ENABLE
#define SYS_FLASH_HANDLER_ENABLE 1     // flash处理器
#define SYS_CONFIG_ENABLE 1            // 配置管理
#define NET_MGR_ENABLE 1               // 网络管理
#define SYS_STATE_ENABLE 1             // 系统状态
#define THING_MODEL_ENABLE 1           // 物联网模型
#define FLASH_HANDLER_HOTPLUG_ENABLE 1 // 启用热拔插轮询
#endif

// 设备 ID 定义
#define FLASH_EXT_SDCARD 3 // 外部SD卡 ID

// 系统使用的存储设备 (可选: FLASH_EXT_QSPI, FLASH_EXT_SDCARD)
#define SYS_FLASH_DEV FLASH_EXT_SDCARD

// 根据存储设备类型定义路径 (FatFS 用盘符, LittleFS 用目录前缀)
#if (SYS_FLASH_DEV == FLASH_EXT_SDCARD)
#define SYS_STORAGE_MOUNT_POINT "0:" // FatFS 盘符
#else
#define SYS_STORAGE_MOUNT_POINT "/sys" // LittleFS 挂载点
#endif

#define SYS_CONFIG_DIR "/config/"         // 配置文件夹路径
#define APP_SETTINGS_DIR "/app_settings/" // 应用设置导出文件夹

#endif /* PROJECT_CFG_H_ */
