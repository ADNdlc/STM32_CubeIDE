/*
 * app.c
 *
 *  Created on: Dec 15, 2025
 *      Author: 12114
 */
#include "project_cfg.h"
#if !TEST_ENABLE && !RES_BURN_ENABLE
#include "app.h"
#define LOG_TAG "APP"
#include "elog.h"

#if !USE_Simulator
#include "home/home.h" // UI
#include "home/System/net_mgr.h" // 网络服务
#include "home/System/cloud_bridge.h" // MQTT服务
#include "home/System/sys_config.h" // 系统配置
#include "home/System/data_logger.h" // 本地数据日志
#include "home/System/sys_state.h" // 系统状态
#include "home/System/sys_power.h" // 电源管理
#include "device_handle.h" // 物模型注册

// VFS & Storage
#include "vfs_manager.h"
#include "storage_factory.h"
#include "strategy/lfs_strategy.h"
#include "strategy/fatfs_strategy.h"
#include "dev_map.h"

#else
#include "virtual_device.h"
#endif

#if LVGL_ENABLE
#include "Colorwheel/colorwheel.h"
#include "device_control/device_control.h"
#include "lvgl.h"
#include "lvgl_resource/lvgl_resource.h"
#endif

int app_init(void) {
#if !USE_Simulator
  // 1. 初始化 VFS 与挂载存储
  log_i("Initializing VFS (Parity with test_settings)...");
  vfs_init();
  
  // 使用 SD 卡挂载到 /sys 路径，采用 FatFS 策略 (此配置已通过测试)
  storage_device_t *sd_storage = storage_factory_get(STORAGE_ID_SD_CARD);
  if (sd_storage) {
      fs_strategy_t *fatfs_strat = fatfs_strategy_create();
      if (vfs_mount("sys", sd_storage, fatfs_strat) == VFS_OK) {
        log_i("Mount /sys (SD Card + FatFS) SUCCESS.");
      } else {
        log_e("Mount /sys FAILED!");
      }
  } else {
      log_e("SD Card storage not found in factory!");
  }

  // 2. 初始化核心系统
  sys_config_init(); // 此时 VFS 已就绪，可以从 /sys 加载配置
  sys_state_init();
  sys_power_init();
  devices_init();
  data_logger_init(); // 日志模块初始化
#endif

#if SERVICE_ENABLE
  net_mgr_init();
  
  // 如果 GUI 未开启，全自动连接网络
#if !GUI_ENABLE && NETWORK_SERVICE_ENABLE
    log_i("GUI disabled, auto-enabling WiFi...");
    net_mgr_wifi_enable(true);
  #endif
#endif

#if LVGL_ENABLE
  res_init();  // 初始化资源管理及处理烧录/显示测试

  home_init(); // 初始化ui核心功能
  UI_Start(); // 启动 UI 系统
  log_i("UI initialized");
#endif

  log_i("Application initialization completed...");
  return 0;
}

void app_run(void) {
  vfs_storage_monitor_task();
#if THINGMODEL_ENABLE
  devices_process(); // 本地设备处理
  data_logger_process(); // 检查并落盘本地日志
#endif
#if SERVICE_ENABLE
  net_mgr_process(); // 网络服务处理
  sys_power_process(); // 电源管理处理
#endif
}
#else
int app_init(void) {(void)0; return 0;}
void app_run(void) {(void)0;}
#endif
