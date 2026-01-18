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
#include "strategy/lfs_strategy.h"
#include "thing_model/thing_model.h"
#include <device_handle.h>
#include <stddef.h>


#define LOG_TAG "SYS_INIT"
// 以下值根据device_mapping定义
#define FLASH_EXT_SPI 0
#define FLASH_EXT_QSPI 1
#define FLASH_EXT_NAND 2
// 定义系统使用的flash设备(主要存储配置信息)
#define SYS_FLASH_DEV FLASH_EXT_QSPI
// 系统配置路径
#define SYS_STORE_MOUNT_POINT "/sys"

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
  // 2.创建LFS策略和对应设备配置
  lfs_strategy_config_t lfs_cfg = {
#if (SYS_FLASH_DEV == FLASH_EXT_NAND)
      .read_size = 2048,
      .prog_size = 2048,
      .cache_size = 2048,
      .lookahead_size = 128, // Increase lookahead for larger device
      .block_cycles = 500,
#elif (SYS_FLASH_DEV == FLASH_EXT_QSPI)
      .read_size = 16,
      .prog_size = 16,
      .cache_size = 256,
      .lookahead_size = 32,
      .block_cycles = 200,
#endif
  };
  log_i("LFS Config: Read=%d, Prog=%d, Cache=%d, Lookahead=%d, Cycles=%d",
        lfs_cfg.read_size, lfs_cfg.prog_size, lfs_cfg.cache_size,
        lfs_cfg.lookahead_size, lfs_cfg.block_cycles);
  flash_strategy_t *lfs_strat = lfs_strategy_create(&lfs_cfg);
  if (!lfs_strat) {
    log_e("LFS Strategy Create Failed");
    return -1;
  }

  // 3. 注册系统资源挂载点
  if (flash_handler_register(SYS_STORE_MOUNT_POINT, sys_dev, lfs_strat) != 0) {
    log_e("LFS Handler Register Failed");
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
