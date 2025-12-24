#ifndef COMPONENT_FLASH_HANDLER_STRATEGY_RAW_STRATEGY_H_
#define COMPONENT_FLASH_HANDLER_STRATEGY_RAW_STRATEGY_H_

#include "flash_strategy.h"

flash_strategy_t *raw_strategy_create(void);
void raw_strategy_destroy(flash_strategy_t *strategy);

#endif /* COMPONENT_FLASH_HANDLER_STRATEGY_RAW_STRATEGY_H_ */
