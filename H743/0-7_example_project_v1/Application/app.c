/*
 * app.c
 *
 *  Created on: Dec 15, 2025
 *      Author: 12114
 */
#include "app.h"
#define LOG_TAG "APP"
#include "elog.h"
#include "project_cfg.h"
#include "sys_config.h"

#if !USE_Simulator
#include "hal_init.h"
#include "sys_init.h"
#include "device_handle.h"

#include "home.h"
#include "home/System/net_mgr.h"
#include "res_burner.h"
#include "res_manager.h"
#endif

#if LVGL_ENABLE
#include "lvgl.h"
#include "Colorwheel/colorwheel.h"
#include "device_control/device_control.h"
#if USE_Simulator
#include "virtual_device.h"
#endif
#endif



int app_init(void) {
#if USE_Simulator   // 模拟器环境
  sys_config_init();
  sys_state_init();
  devices_init();   // 注册模拟设备
#endif
#if CONFIG_RES_BURN_ENABLE
  sys_delay_ms(1000);
  res_burner_run();
  log_i("Resource burning completed. Please disable CONFIG_RES_BURN_ENABLE and "
        "re-flash.");
  while (1) {
    sys_delay_ms(1000); // 烧录模式下不进入主循环
  }
#endif

#if LVGL_ENABLE && !CONFIG_RES_BURN_ENABLE
  home_init();        // 初始化ui核心功能

  /* 注册各ui模块功能 */
  colorwheel_app_register(0);     // 注册 ColorWheel
  device_control_app_register(0); // 注册 Control

  ui_Start(); // 启动 UI 系统
  log_i("New UI initialized");
#endif

  log_i("Application initialization completed...");
  return 0;
}

void app_run(void) {
#if !CONFIG_RES_BURN_ENABLE
#if !USE_Simulator
  sys_devices_process();  // 本地设备处理
  net_mgr_process();  // 网络服务处理
#endif
#if LVGL_ENABLE
  lv_timer_handler(); // ui处理
#endif
#endif
}
