#include "sys_init.h"
#include "cloud_bridge.h"
#include "elog.h"
#include "factory/flash_factory.h"
#include "flash_handler.h"
#include "home/System/net_mgr.h"
#include "home/System/sys_config.h"
#include "home/System/sys_state.h"
#include "lv_port_fs.h"
#include "mqtt_service.h"
#include "project_cfg.h"
#include "rtc_factory.h"
#include "rtc_hal/rtc_hal.h"
#include "strategy/fatfs_strategy.h"
#include "strategy/lfs_strategy.h"
#include "thing_model/thing_model.h"
#include <device_handle.h>
#include <stddef.h>

#define LOG_TAG "SYS_INIT"
// 以下值根据device_mapping定义
#define FLASH_EXT_SPI 0
#define FLASH_EXT_QSPI 1
#define FLASH_EXT_NAND 2
// 系统配置路径 (从 project_cfg.h 获取统一映射)
#ifndef SYS_STORAGE_MOUNT_POINT
#define SYS_STORAGE_MOUNT_POINT "/sys"
#endif

int sys_services_init(void) {
  log_i("Initializing system services...");

#if SYS_FLASH_HANDLER_ENABLE && !CONFIG_RES_BURN_ENABLE

  /*****************
   * 存储和文件系统
   *****************/

  flash_handler_init(); // 初始化 Flash 管理器

  // 1.获取存储设备
  block_device_t *sys_dev = flash_factory_get(SYS_FLASH_DEV);
  if (!sys_dev) {
    log_e("Failed to get system storage device.");
    return -1;
  }

  // 3. 根据类型选择策略并在系统资源挂载点注册
  flash_strategy_t *sys_strat = NULL;
  block_dev_info_t dev_info;
  BLOCK_DEV_GET_INFO(sys_dev, &dev_info);

#if (SYS_FLASH_DEV == FLASH_EXT_SDCARD)
  // SD卡使用 FatFS 策略
  fatfs_strategy_config_t fat_cfg = {.pdrv = 0}; // 默认为盘符0
  sys_strat = fatfs_strategy_create(&fat_cfg);
  flash_device_type_t dev_type = FLASH_TYPE_POLLING;
#else
  // 其他（SPI/QSPI）使用 LittleFS 策略
  lfs_strategy_config_t lfs_cfg = {
      .read_size = 16,
      .prog_size = 16,
      .cache_size = 256,
      .lookahead_size = 32,
      .block_cycles = 200,
  };
  sys_strat = lfs_strategy_create(&lfs_cfg);
  flash_device_type_t dev_type = FLASH_TYPE_STATIC;
#endif

  if (!sys_strat) {
    log_e("Flash Strategy Create Failed");
    return -1;
  }

  if (flash_handler_register(SYS_STORAGE_MOUNT_POINT, sys_dev, sys_strat,
                             dev_type) != 0) {
    log_e("Flash Handler Register Failed");
    return -1;
  }

  /* lvgl文件系统初始化 */
#if LVGL_ENABLE && LVGL_FS_INIT && !CONFIG_RES_BURN_ENABLE
  // lv_port_fs_init(); // 目前lvgl关联/lfs挂载点
#endif
#endif

  /*****************
   * 系统服务和组件
   *****************/
#if SYS_STATE_ENABLE
  sys_state_init(); // 初始化系统状态
#endif
#if THING_MODEL_ENABLE
  thing_model_init(); // 初始化物模型管理器
  sys_devices_init(); // 初始化并注册硬件设备
#endif
#if NET_MGR_ENABLE
  net_mgr_init(); // 初始化网络管理器
#endif

  // RTC 初始化
  rtc_driver_t *rtc_drv = rtc_driver_get(RTC_DEVICE_0);
  if (rtc_drv) {
    rtc_hal_init(rtc_drv, NULL, NULL);
  } else {
    log_w("RTC driver not found, skipping RTC HAL init.");
  }

  log_i("System services initialization completed.");
  return 0;
}
