/*
 * flash_dependencies.h
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  Flash 驱动依赖注入接口
 *  定义驱动所需的全部外部依赖（裸机环境）
 */

#ifndef INTERFACE_FLASH_DEPENDENCIES_H_
#define INTERFACE_FLASH_DEPENDENCIES_H_

#include "spi_adapter.h"
#include <stdarg.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

// 日志函数类型（匹配elog）
typedef void (*flash_log_fn)(const char *tag, const char *fmt, ...);

// Flash 驱动依赖结构体
typedef struct {
  // SPI 通信依赖
  spi_adapter_t *spi_adapter;

  // 系统依赖
  void (*delay_ms)(uint32_t ms); // 毫秒延时（使用 sys_delay_ms）
  void (*delay_us)(uint32_t us); // 微秒延时（使用 sys_delay_us，可选）

  // 日志依赖（使用 EasyLogger）
  flash_log_fn log_debug; // elog_d
  flash_log_fn log_info;  // elog_i
  flash_log_fn log_warn;  // elog_w
  flash_log_fn log_error; // elog_e

} flash_dependencies_t;

// 验证依赖是否完整
// 返回: 0 = 完整，其他 = 缺少必要依赖
int flash_dependencies_validate(const flash_dependencies_t *deps);

// 创建默认依赖（需平台实现）
// 该函数由平台层实现，返回平台特定的依赖配置
flash_dependencies_t *flash_dependencies_create_default(void);

// 销毁依赖
void flash_dependencies_destroy(flash_dependencies_t *deps);

// ============== 日志辅助宏 ==============
// 使用依赖中的日志函数，自动传递TAG

#define FLASH_LOG_TAG "FLASH"

#define FLASH_LOG_D(deps, fmt, ...)                                            \
  do {                                                                         \
    if ((deps) && (deps)->log_debug)                                           \
      (deps)->log_debug(FLASH_LOG_TAG, fmt, ##__VA_ARGS__);                    \
  } while (0)

#define FLASH_LOG_I(deps, fmt, ...)                                            \
  do {                                                                         \
    if ((deps) && (deps)->log_info)                                            \
      (deps)->log_info(FLASH_LOG_TAG, fmt, ##__VA_ARGS__);                     \
  } while (0)

#define FLASH_LOG_W(deps, fmt, ...)                                            \
  do {                                                                         \
    if ((deps) && (deps)->log_warn)                                            \
      (deps)->log_warn(FLASH_LOG_TAG, fmt, ##__VA_ARGS__);                     \
  } while (0)

#define FLASH_LOG_E(deps, fmt, ...)                                            \
  do {                                                                         \
    if ((deps) && (deps)->log_error)                                           \
      (deps)->log_error(FLASH_LOG_TAG, fmt, ##__VA_ARGS__);                    \
  } while (0)

// 延时辅助宏
#define FLASH_DELAY_MS(deps, ms)                                               \
  do {                                                                         \
    if ((deps) && (deps)->delay_ms)                                            \
      (deps)->delay_ms(ms);                                                    \
  } while (0)

#define FLASH_DELAY_US(deps, us)                                               \
  do {                                                                         \
    if ((deps) && (deps)->delay_us)                                            \
      (deps)->delay_us(us);                                                    \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif /* INTERFACE_FLASH_DEPENDENCIES_H_ */
