#ifndef COMPONENT_FLASH_HANDLER_STRATEGY_LFS_STRATEGY_H_
#define COMPONENT_FLASH_HANDLER_STRATEGY_LFS_STRATEGY_H_

#include "../fs_strategy.h"
#include "lfs.h"

// 新增设备操作信息
typedef struct {
  lfs_size_t read_size;
  lfs_size_t prog_size;
  lfs_size_t cache_size;
  lfs_size_t lookahead_size;
  lfs_size_t block_cycles;
} lfs_strategy_config_t;

typedef struct lfs_strategy_t {
  fs_strategy_t Base;
  lfs_t lfs;            // LittleFS 实例
  struct lfs_config cfg;// LittleFS 配置
} lfs_strategy_t;


/**
 * @brief Create a LittleFS Strategy instance
 *
 * @return flash_strategy_t* Pointer to the created strategy
 */
fs_strategy_t *lfs_strategy_create(const lfs_strategy_config_t *config);

/**
 * @brief Destroy a LittleFS Strategy instance
 *
 * @param strategy Pointer to the strategy to destroy
 */
void lfs_strategy_destroy(lfs_strategy_t *strategy);

#endif /* COMPONENT_FLASH_HANDLER_STRATEGY_LFS_STRATEGY_H_ */
