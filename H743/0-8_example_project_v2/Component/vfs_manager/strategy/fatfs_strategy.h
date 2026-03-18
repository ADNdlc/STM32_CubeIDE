#ifndef COMPONENT_FLASH_HANDLER_STRATEGY_FATFS_STRATEGY_H_
#define COMPONENT_FLASH_HANDLER_STRATEGY_FATFS_STRATEGY_H_

#include "../fs_strategy.h"
#include "ff.h"


typedef struct fatfs_strategy_t {
  fs_strategy_t *Base;
  FATFS fs;		// fatFS 实例
} fatfs_strategy_t;

fs_strategy_t *fatfs_strategy_create(void);

void fatfs_strategy_destroy(fatfs_strategy_t *strategy);

#endif /* COMPONENT_FLASH_HANDLER_STRATEGY_FATFS_STRATEGY_H_ */
