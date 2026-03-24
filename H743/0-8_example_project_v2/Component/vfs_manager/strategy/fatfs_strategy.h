#ifndef COMPONENT_FLASH_HANDLER_STRATEGY_FATFS_STRATEGY_H_
#define COMPONENT_FLASH_HANDLER_STRATEGY_FATFS_STRATEGY_H_

#include "../fs_strategy.h"
#include "ff.h"


typedef struct fatfs_strategy_t {
    fs_strategy_t base;     // 必须是第一个成员！方便直接指针强转
    FATFS fs;               // FatFs 实例工作区
    char drive_num[4];      // 绑定的逻辑盘符，如 "0:" 或 "1:"
    uint8_t pdrv;           // 传递给 diskio.c 的物理驱动器号
} fatfs_strategy_t;

fs_strategy_t *fatfs_strategy_create(void);

void fatfs_strategy_destroy(fatfs_strategy_t *strategy);

#endif /* COMPONENT_FLASH_HANDLER_STRATEGY_FATFS_STRATEGY_H_ */
