/*
 * BSP_init.c
 *
 *  Created on: Feb 7, 2026
 *      Author: 12114
 */
#include "dev_map_config.h"
#if (TARGET_PLATFORM == PLATFORM_STM32)
#include "stm32_mem.h"
#define platform_sys_mem_create()                                              \
  stm32_sys_mem_create() // sys无工厂，简单使用宏来绑定
#endif                   // TARGET_PLATFORM
#include "MemPool.h"
#include "Sys.h"
#include "dev_map.h"
#include "elog_init.h"
#include "humiture_factory.h"
#include "lvgl.h"
#include "lvgl/porting/lv_port_disp.h"
#include "sdram_factory.h"
#include "timer_factory.h"
#include "uart_queue.h"
#include "usart_factory.h"

/* 全局设备句柄 */
uart_queue_t *g_debug_queue = NULL;

/* 调试串口队列用的缓冲区 */
static uint8_t debug_tx_buf[4096];
static uint8_t debug_rx_buf[64];

/* LVGL相关 */
lcd_driver_t *lcd = NULL;
static void lv_tick_cb(void *ctx) { lv_tick_inc(1); } //供 Timer 回调使用的 LVGL 心跳

void bsp_init(void) {
#ifndef platform_sys_mem_create
#error "bsp_init: 未定义有效的目标平台，请检查TARGET_PLATFORM配置"
#endif
  SysMem *mem_temp = NULL;
  mem_temp = platform_sys_mem_create();
  sys_mem_init(mem_temp); // 内存池功能初始化
  /* 系统初始化 */
  sys_init();
  sys_mem_init_internal();

  /* 1. 创建硬件抽象层串口对象 */
  usart_driver_t *g_debug_uart = usart_driver_get(USART_ID_DEBUG);
  /* 2. 创建并初始化日志串口队列 */
  if (g_debug_uart) {
    g_debug_queue =
        uart_queue_create(g_debug_uart, debug_tx_buf, sizeof(debug_tx_buf),
                          debug_rx_buf, sizeof(debug_rx_buf));
    if (g_debug_queue) {
      uart_queue_set_wait_config(g_debug_queue, 2, 50); // 2ms等待, 最多50次
      uart_queue_set_auto_wait(g_debug_queue, true);
      uart_queue_start_receive(g_debug_queue); // 开启异步接收
    }
  }
  /* 3. elog初始化 */
  if (elog_init_and_config() == ELOG_NO_ERR) {
    log_i("log init success.");
    log_a("log lvel: %d", ELOG_LVL_TOTAL_NUM);
    log_i("sys CoreClock: %d MHz", sys_get_CoreClock());
  } else {
    elog_deinit();
  }

  /* ---- 外部sdram和内存池初始化,注意顺序 ---- */
  sdram_driver_t *driver = sdram_driver_get(SDRAM_MAIN); // 获取SDRAM驱动
  if (driver == NULL) {
    log_e("Failed to get SDRAM driver instance");
    while (1) {
    }
  }
  if (SDRAM_INIT(driver) != 0) { // SDRAM初始化
    log_e("Failed to initialize SDRAM device");
    while (1) {
    }
  }
  sys_mem_init_external(); // 外部内存池初始化

  /* ---- LVGL 初始化 ---- */
  lv_init();
  lv_port_disp_init();
  log_i("LVGL core init success.");

  /* ---- LVGL 定时器触发 (时基) ---- */
  timer_driver_t *timer_lv = timer_driver_get(TIMER_ID_LV);
  if (timer_lv) {
    TIMER_SET_PERIOD(timer_lv, 1);
    TIMER_SET_CALLBACK(timer_lv, lv_tick_cb, NULL);
    TIMER_START(timer_lv);
    log_i("LVGL timer tick starte(1ms).");
  } else {
    log_e("LVGL timer tick init failed!");
  }
}
