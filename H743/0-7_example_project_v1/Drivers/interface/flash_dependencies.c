/*
 * flash_dependencies.c
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  Flash 驱动依赖注入实现
 */

#include "flash_dependencies.h"
#include <stddef.h>

// 验证依赖是否完整
int flash_dependencies_validate(const flash_dependencies_t *deps) {
  if (deps == NULL) {
    return -1;
  }

  // SPI 适配器是必须的
  if (deps->spi_adapter == NULL) {
    return -2;
  }

  // 延时函数是必须的
  if (deps->delay_ms == NULL) {
    return -3;
  }

  // 日志函数是可选的，不验证

  return 0;
}

// 销毁依赖（基础实现）
// 注意：如果依赖是静态分配的，此函数不做任何事
// 平台层可以覆盖此实现
__attribute__((weak)) void
flash_dependencies_destroy(flash_dependencies_t *deps) {
  (void)deps;
  // 默认实现：什么都不做
  // 平台层如果动态分配了依赖，需要覆盖此函数
}
