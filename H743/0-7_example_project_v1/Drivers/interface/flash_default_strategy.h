/*
 * flash_default_strategy.h
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  默认存储策略（直通式读写）
 */

#ifndef INTERFACE_FLASH_DEFAULT_STRATEGY_H_
#define INTERFACE_FLASH_DEFAULT_STRATEGY_H_

#include "flash_strategy.h"

#ifdef __cplusplus
extern "C" {
#endif

// 创建默认策略实例
flash_strategy_t *flash_default_strategy_create(void);

// 销毁默认策略实例
void flash_default_strategy_destroy(flash_strategy_t *strategy);

#ifdef __cplusplus
}
#endif

#endif /* INTERFACE_FLASH_DEFAULT_STRATEGY_H_ */
