#include "sys_init.h"
#include "elog.h"
#include "factory/flash_factory.h"
#include "flash_handler.h"
#include "home/System/net_mgr.h"
#include "home/System/sys_state.h"
#include "lv_port_fs.h"
#include "project_cfg.h"
#include "strategy/lfs_strategy.h"
#include "w25qxx/w25q_adapter.h"
#include "w25qxx/w25qxx.h"
#include <stddef.h>

#define LOG_TAG "SYS_INIT"

int sys_services_init(void) {
  log_i("Initializing system services...");

  /* 存储和文件系统 */
  flash_handler_init(); // 初始化 Flash 管理器

#if !CONFIG_RES_BURN_ENABLE
  // --- 运行模式 (Runtime XIP Mode) ---
  log_i("Mode: RUNTIME (XIP Read-Only)");

  // 1. 切换到内存映射模式
  block_device_t *dev = flash_factory_get(FLASH_EXT_QSPI);
  w25q_adapter_t *qspi_adapter = ((w25qxx_t *)dev)->adapter;
  if (dev && qspi_adapter && qspi_adapter->ops->enter_memory_mapped) {
    if (qspi_adapter->ops->enter_memory_mapped(qspi_adapter) != 0) {
      log_e("Failed to enter QSPI memory mapped mode");
      return -1;
    }
    log_i("QSPI switched to Memory Mapped Mode (0x90000000)");
  }

  // 2. 初始化核心服务 (无文件系统，将使用默认配置)
  sys_state_init();
  net_mgr_init();
#endif

  log_i("System services initialization completed.");
  return 0;
}
