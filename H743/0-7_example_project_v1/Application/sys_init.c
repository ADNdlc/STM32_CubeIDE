#include "sys_init.h"
#include "elog.h"
#include "factory/flash_factory.h"
#include "flash_handler.h"
#include "home/System/net_mgr.h"
#include "home/System/sys_config.h"
#include "home/System/sys_state.h"
#include "lv_port_fs.h"
#include "project_cfg.h"
#include "strategy/lfs_strategy.h"
#include <stddef.h>

#define LOG_TAG "SYS_INIT"
// 以下值根据device_mapping定义
#define FLASH_EXT_SPI 0
#define FLASH_EXT_QSPI 1
#define FLASH_EXT_NAND 2
#define SYS_FLASH_DEV FLASH_EXT_QSPI
#define SYS_STORE_MOUNT_POINT "/sys"

int sys_services_init(void) {
  log_i("Initializing system services...");

#if !CONFIG_RES_BURN_ENABLE
  /* 存储和文件系统 */
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

  // 4.读取信息验证系统文件完整性
  log_i("System storage mounted at %s", SYS_STORE_MOUNT_POINT);

  /* lvgl文件系统初始化 */
#if LVGL_FS_INIT && !CONFIG_RES_BURN_ENABLE
  // lv_port_fs_init(); // 目前lvgl关联/lfs挂载点
#endif

  /* 系统组件初始化 */
  sys_config_init(); // 初始化系统配置 (从 Flash 加载)
  sys_state_init();  // 初始化系统状态
  net_mgr_init();    // 初始化网络管理器

#endif
  log_i("System services initialization completed.");
  return 0;
}
