#include "fatfs_strategy.h"
#include "MemPool.h"
#include "elog.h"

#define LOG_TAG "FATFS_STRAT"
#define FATFS_STRATEGY_MEMSOURCE SYS_MEM_INTERNAL

static const fs_ops_t fatfs_ops = {

};

fs_strategy_t *fatfs_strategy_create(void) {

}

void fatfs_strategy_destroy(fatfs_strategy_t *strategy) {

}
