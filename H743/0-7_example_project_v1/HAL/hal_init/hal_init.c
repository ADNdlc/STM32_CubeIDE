/*
 * hal_init.c
 *
 *  Created on: Dec 12, 2025
 *      Author: 12114
 */

#include "hal_init.h"
#include "cJSON.h"
#include "sys.h"

static void *cjson_malloc(size_t sz) {
  return sys_malloc(SYS_MEM_INTERNAL, (uint32_t)sz);
}

static void cjson_free(void *ptr) { sys_free(SYS_MEM_INTERNAL, ptr); }

int hal_init(void) {
  /* ---- SYS初始化 ---- */
  sys_bind_ops(SysFactory_GetOps()); // 绑定平台sys操作
  sys_core_init();                   // 基本操作初始化
  sys_mem_init_internal();           // 内存池初始化

  /* ---- elog初始化 ---- */
  if (elog_init_and_config() == ELOG_NO_ERR) {
    log_a("This is an ASSERT message.");
    log_e("This is an ERROR message.");
    log_w("This is a WARNING message.");
    log_i("This is an INFO message.");
    log_d("This is a DEBUG message.");
    log_v("This is a VERBOSE message.");
  } else {
    elog_deinit();
  }

  /* ---- 外部sdram和内存池初始化,注意顺序 ---- */
  sdram_driver_t *driver = sdram_driver_get(SDRAM_MAIN);
  if (driver == NULL) {
    log_e("Failed to get SDRAM driver instance");
    while (1) {
    }
  }
  if (SDRAM_INIT(driver) != 0) {
    log_e("Failed to initialize SDRAM device");
    while (1) {
    }
  }
  // 内存池
  sys_mem_init_external();

  // 配置 cJSON 全局内存钩子，确保所有模块使用相同的堆
  cJSON_Hooks hooks = {.malloc_fn = cjson_malloc, .free_fn = cjson_free};
  cJSON_InitHooks(&hooks);

  log_i("SDRAM&MEM initialization completed...");

#if LVGL_ENABLE
  /* ---- LVGL硬件初始化 ---- */
  lv_init(); // LVGL 初始化
  log_i("LVGL initialization completed...");
#if LVGL_DISP_INIT && !CONFIG_RES_BURN_ENABLE
  lv_port_disp_init(); // 注册LVGL的显示任务
  log_i("LVGL port_disp initialization completed...");
#endif
#if LVGL_INDEV_INIT && !CONFIG_RES_BURN_ENABLE
  lv_port_indev_init(); // 注册LVGL的触屏检测任务
  log_i("LVGL port_indev initialization completed...");
#endif
#endif
  /* --------------------- */

  log_i("HAL initialization completed...");
  return 0;
}
