/*
 * app.c
 *
 *  Created on: Dec 15, 2025
 *      Author: 12114
 */
#include "project_cfg.h"
#include "app.h"
#include "elog.h"
#define LOG_TAG "APP"

#if !USE_Simulator
#include "home/home.h" // UI
#include "home/System/net_mgr.h" // 网络服务
#include "home/System/cloud_bridge.h" // MQTT服务
#include "device_handle.h" // 物模型注册
#else
#include "virtual_device.h"
#endif

#if LVGL_ENABLE
#include "Colorwheel/colorwheel.h"
#include "device_control/device_control.h"
#include "lvgl.h"
#endif

int app_init(void) {
#if USE_Simulator // 模拟器环境
  sys_config_init();
  sys_state_init();
  devices_init(); // 注册模拟设备
#endif

#if SERVICE_ENABLE
  net_mgr_init();
#endif

#if LVGL_ENABLE
  home_init(); // 初始化ui核心功能

  /* 注册各ui模块 */
  colorwheel_app_register(0);     // 注册 ColorWheel
  device_control_app_register(0); // 注册 Control

  ui_Start(); // 启动 UI 系统
  log_i("UI initialized");
#endif

  log_i("Application initialization completed...");
  return 0;
}

void app_run(void) {

  sys_devices_process(); // 本地设备处理

  net_mgr_process(); // 网络服务处理

#if LVGL_ENABLE
  lv_timer_handler(); // ui处理
#endif

}
