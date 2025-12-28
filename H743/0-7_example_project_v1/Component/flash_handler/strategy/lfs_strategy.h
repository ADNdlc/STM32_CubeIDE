#ifndef COMPONENT_FLASH_HANDLER_STRATEGY_LFS_STRATEGY_H_
#define COMPONENT_FLASH_HANDLER_STRATEGY_LFS_STRATEGY_H_

#include "flash_strategy.h"

/**
 * @brief Create a LittleFS Strategy instance
 *
 * @return flash_strategy_t* Pointer to the created strategy
 */
flash_strategy_t *lfs_strategy_create(void);

/**
 * @brief Destroy a LittleFS Strategy instance
 *
 * @param strategy Pointer to the strategy to destroy
 */
void lfs_strategy_destroy(flash_strategy_t *strategy);
struct lfs *lfs_strategy_get_lfs(flash_strategy_t *strategy);

#endif /* COMPONENT_FLASH_HANDLER_STRATEGY_LFS_STRATEGY_H_ */
