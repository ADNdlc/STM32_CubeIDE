/*
 * app.c
 *
 *  Created on: Dec 15, 2025
 *      Author: 12114
 */

#define USE_OLD_UI 0

#include "app.h"
#include "lvgl.h"
#include "elog.h"

#define LOG_TAG "APP"
#include "elog.h"

#if USE_OLD_UI
#include "ui/Act_Manager.h"
#else
#include "home.h"
#include "Colorwheel/colorwheel.h"
#include "device_control/device_control.h"
#endif

int app_init(void)
{
#if USE_OLD_UI
  act_manager_init(); // 旧ui入口
  log_i("Old UI initialized");
#else
  home_init();  // 初始化核心功能

  /* 注册各模块功能 */
  colorwheel_app_register(0);		// 注册 ColorWheel
  device_control_app_register(0);	// 注册 Control

  ui_Start();  // 启动 UI 系统
  log_i("New UI initialized");
#endif
  return 0;
}

void app_run(void)
{
  lv_timer_handler();
}
