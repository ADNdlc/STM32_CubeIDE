/*
 * app.c
 *
 *  Created on: Dec 15, 2025
 *      Author: 12114
 */

#define USE_OLD_UI 0

#include "app.h"
#define LOG_TAG "APP"
#include "elog.h"
#include "hal_init.h"

#if LVGL_INIT
#include "home/System/net_mgr.h"
#include "lvgl.h"

#if USE_OLD_UI
#include "ui/Act_Manager.h"
#else
#include "Colorwheel/colorwheel.h"
#include "device_control/device_control.h"
#include "home.h"

#endif
#endif

#include "home/res_burner.h"
#include "sys_init.h"
#define CONFIG_RES_BURN_ENABLE 0 // 设置为1时开启烧录模式，烧录完成后应设回0
 
int app_init(void) {
  // 系统服务和组件初始化
  sys_services_init();

#if CONFIG_RES_BURN_ENABLE
  res_burner_run();
  log_i("Resource burning completed. Please disable CONFIG_RES_BURN_ENABLE and "
        "re-flash.");
  while (1) {
    sys_delay_ms(1000); // 烧录模式下不进入主循环
  }
#endif

#if LVGL_INIT && !CONFIG_RES_BURN_ENABLE
#if USE_OLD_UI
  act_manager_init(); // 旧ui入口
  log_i("Old UI initialized");
#else
  home_init(); // 初始化ui核心功能

  /* 注册各ui模块功能 */
  colorwheel_app_register(0);     // 注册 ColorWheel
  device_control_app_register(0); // 注册 Control

  ui_Start(); // 启动 UI 系统
  log_i("New UI initialized");
#endif
#endif

  log_i("Application initialization completed...");
  return 0;
}

void app_run(void) {
#if LVGL_INIT && !CONFIG_RES_BURN_ENABLE
  lv_timer_handler();
  net_mgr_process();
#endif
}
