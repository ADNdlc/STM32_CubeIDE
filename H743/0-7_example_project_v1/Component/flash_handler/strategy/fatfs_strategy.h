#ifndef COMPONENT_FLASH_HANDLER_STRATEGY_FATFS_STRATEGY_H_
#define COMPONENT_FLASH_HANDLER_STRATEGY_FATFS_STRATEGY_H_

#include "flash_strategy.h"

typedef struct {
  uint8_t pdrv; // Physical drive number (0-9)
} fatfs_strategy_config_t;

/**
 * @brief Create a FatFS Strategy instance
 *
 * @param config Pointer to the configuration
 * @return flash_strategy_t* Pointer to the created strategy
 */
flash_strategy_t *fatfs_strategy_create(const fatfs_strategy_config_t *config);

/**
 * @brief Destroy a FatFS Strategy instance
 *
 * @param strategy Pointer to the strategy to destroy
 */
void fatfs_strategy_destroy(flash_strategy_t *strategy);

#endif /* COMPONENT_FLASH_HANDLER_STRATEGY_FATFS_STRATEGY_H_ */
