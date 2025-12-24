/*
 * flash_strategy.h
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  Flash 存储策略接口（策略模式）
 *  支持不同的存储管理算法（直接读写、FatFS等）
 */

#ifndef INTERFACE_FLASH_STRATEGY_H_
#define INTERFACE_FLASH_STRATEGY_H_

#include "flash_driver.h"
#include <stddef.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

// 前向声明
typedef struct flash_strategy_t flash_strategy_t;

// 策略类型
typedef enum {
  FLASH_STRATEGY_RAW = 0,  // 原始读写（直通式）
  FLASH_STRATEGY_FATFS,    // FatFS 文件系统
  FLASH_STRATEGY_LITTLEFS, // LittleFS 文件系统
  FLASH_STRATEGY_CUSTOM,   // 自定义策略
} flash_strategy_type_t;

// 策略操作接口
typedef struct {
  // 初始化策略
  flash_error_t (*init)(flash_strategy_t *self, flash_driver_t *driver);

  // 去初始化
  flash_error_t (*deinit)(flash_strategy_t *self);

  // 读取数据
  flash_error_t (*read)(flash_strategy_t *self, uint32_t addr, uint8_t *buf,
                        uint32_t len);

  // 写入数据（策略可能会自动处理擦除）
  flash_error_t (*write)(flash_strategy_t *self, uint32_t addr,
                         const uint8_t *buf, uint32_t len);

  // 擦除区域
  flash_error_t (*erase)(flash_strategy_t *self, uint32_t addr, uint32_t len);

  // 同步刷新（将缓存写入Flash）
  flash_error_t (*sync)(flash_strategy_t *self);

  // 获取可用空间
  uint32_t (*get_free_space)(flash_strategy_t *self);

  // 获取已用空间
  uint32_t (*get_used_space)(flash_strategy_t *self);

} flash_strategy_ops_t;

// 策略基类
struct flash_strategy_t {
  const flash_strategy_ops_t *ops; // 操作接口
  flash_strategy_type_t type;      // 策略类型
  flash_driver_t *driver;          // 关联的驱动
  void *priv;                      // 私有数据
  const char *name;                // 策略名称
};

// ============== 辅助宏定义 ==============

#define STRATEGY_INIT(s, drv) ((s)->ops->init(s, drv))
#define STRATEGY_DEINIT(s) ((s)->ops->deinit(s))

#define STRATEGY_READ(s, addr, buf, len) ((s)->ops->read(s, addr, buf, len))

#define STRATEGY_WRITE(s, addr, buf, len) ((s)->ops->write(s, addr, buf, len))

#define STRATEGY_ERASE(s, addr, len) ((s)->ops->erase(s, addr, len))

#define STRATEGY_SYNC(s) ((s)->ops->sync(s))

#define STRATEGY_GET_FREE(s) ((s)->ops->get_free_space(s))
#define STRATEGY_GET_USED(s) ((s)->ops->get_used_space(s))

#ifdef __cplusplus
}
#endif

#endif /* INTERFACE_FLASH_STRATEGY_H_ */
