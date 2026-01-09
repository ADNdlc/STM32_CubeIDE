#ifndef COMPONENT_FLASH_HANDLER_STRATEGY_LFS_STRATEGY_H_
#define COMPONENT_FLASH_HANDLER_STRATEGY_LFS_STRATEGY_H_

#include "flash_strategy.h"
#include "lfs.h"

// 新增设备操作信息
typedef struct {
  lfs_size_t read_size;      // 若为0，自动使用 dev->read_unit
  lfs_size_t prog_size;      // 若为0，自动使用 dev->prog_unit
  lfs_size_t cache_size;     // 若为0，自动设定为 prog_size
  lfs_size_t lookahead_size; // 若为0，默认 32
  lfs_size_t block_cycles;   // 默认 500
} lfs_strategy_config_t;

/**
 * @brief Create a LittleFS Strategy instance
 *
 * @return flash_strategy_t* Pointer to the created strategy
 */
flash_strategy_t *lfs_strategy_create(const lfs_strategy_config_t *config);

/**
 * @brief Destroy a LittleFS Strategy instance
 *
 * @param strategy Pointer to the strategy to destroy
 */
void lfs_strategy_destroy(flash_strategy_t *strategy);
struct lfs *lfs_strategy_get_lfs(flash_strategy_t *strategy);

#endif /* COMPONENT_FLASH_HANDLER_STRATEGY_LFS_STRATEGY_H_ */
