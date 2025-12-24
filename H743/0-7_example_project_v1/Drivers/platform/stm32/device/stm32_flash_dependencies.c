/*
 * stm32_flash_dependencies.c
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  STM32 平台 Flash 驱动依赖注入实现
 */

#include "stm32_flash_dependencies.h"
#include "elog.h"
#include "sys.h"
#include <string.h>


// 静态依赖实例
static flash_dependencies_t g_stm32_deps;
static uint8_t g_deps_created = 0;

// ============== 系统延时包装 ==============

static void stm32_delay_ms(uint32_t ms) { sys_delay_ms(ms); }

static void stm32_delay_us(uint32_t us) { sys_delay_us(us); }

// ============== 日志包装（匹配 elog 接口） ==============

// 注意：elog 的宏是变参的，但我们需要函数指针
// 这里使用简化的包装，直接调用 elog 函数

static void stm32_log_debug(const char *tag, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  // elog 的 raw 输出（简化处理）
  // 实际使用时可以换成更完整的实现
  elog_output(ELOG_LVL_DEBUG, tag, NULL, NULL, 0, fmt, args);
  va_end(args);
}

static void stm32_log_info(const char *tag, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  elog_output(ELOG_LVL_INFO, tag, NULL, NULL, 0, fmt, args);
  va_end(args);
}

static void stm32_log_warn(const char *tag, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  elog_output(ELOG_LVL_WARN, tag, NULL, NULL, 0, fmt, args);
  va_end(args);
}

static void stm32_log_error(const char *tag, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  elog_output(ELOG_LVL_ERROR, tag, NULL, NULL, 0, fmt, args);
  va_end(args);
}

// ============== 公共接口 ==============

flash_dependencies_t *stm32_flash_create_dependencies(void) {
  if (g_deps_created) {
    return &g_stm32_deps;
  }

  memset(&g_stm32_deps, 0, sizeof(g_stm32_deps));

  // 系统函数
  g_stm32_deps.delay_ms = stm32_delay_ms;
  g_stm32_deps.delay_us = stm32_delay_us;

  // 日志函数
  g_stm32_deps.log_debug = stm32_log_debug;
  g_stm32_deps.log_info = stm32_log_info;
  g_stm32_deps.log_warn = stm32_log_warn;
  g_stm32_deps.log_error = stm32_log_error;

  // SPI 适配器由工厂设置
  g_stm32_deps.spi_adapter = NULL;

  g_deps_created = 1;

  return &g_stm32_deps;
}

void stm32_flash_destroy_dependencies(flash_dependencies_t *deps) {
  if (deps == &g_stm32_deps) {
    // 静态实例，不释放
    g_deps_created = 0;
  }
}

// 弱符号实现，供 flash_dependencies.c 调用
__attribute__((weak)) flash_dependencies_t *
flash_dependencies_create_default(void) {
  return stm32_flash_create_dependencies();
}
