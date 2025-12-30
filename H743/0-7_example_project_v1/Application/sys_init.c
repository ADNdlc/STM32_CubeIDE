#include "sys_init.h"
#include "elog.h"
#include "factory/flash_factory.h"
#include "flash_handler.h"
#include "home/System/net_mgr.h"
#include "home/System/sys_state.h"
#include "lv_port_fs.h"
#include "project_cfg.h"
#include "strategy/lfs_strategy.h"
#include <stddef.h>

#define LOG_TAG "SYS_INIT"

int sys_services_init(void) {
  log_i("Initializing system services...");

  /* 存储和文件系统 */
  flash_handler_init(); // 初始化 Flash 管理器


  /* lvgl文件系统初始化 */
#if LVGL_FS_INIT && !CONFIG_RES_BURN_ENABLE
  lv_port_fs_init(); // 目前lvgl关联/lfs挂载点
#endif

  /* 系统组件初始化 */
#if !CONFIG_RES_BURN_ENABLE
  sys_state_init(); // 初始化系统状态
  net_mgr_init();   // 初始化网络管理器
#endif

  log_i("System services initialization completed.");
  return 0;
}
