/*
 * flash_handler.h
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  Flash Handler（桥接模式 + 注册器模式）
 *  统一管理多个 Flash 设备，支持策略切换
 */

#ifndef INTERFACE_FLASH_HANDLER_H_
#define INTERFACE_FLASH_HANDLER_H_

#include "flash_dependencies.h"
#include "flash_driver.h"
#include "flash_strategy.h"
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

// 最大支持的 Flash 设备数量
#ifndef MAX_FLASH_DEVICES
#define MAX_FLASH_DEVICES 4
#endif

// 设备名称最大长度
#define FLASH_DEVICE_NAME_MAX 16

// 设备注册信息
typedef struct {
  flash_driver_t *driver;           // 驱动实例
  char name[FLASH_DEVICE_NAME_MAX]; // 设备名称
  uint8_t in_use;                   // 是否在使用
} flash_device_entry_t;

// Flash Handler 结构体
typedef struct {
  flash_device_entry_t devices[MAX_FLASH_DEVICES]; // 注册的设备
  uint8_t device_count;                            // 当前设备数量
  flash_strategy_t *strategy;                      // 当前策略（可选）
  flash_dependencies_t *deps;                      // 依赖
  uint8_t initialized;                             // 初始化标志
} flash_handler_t;

// ============== 初始化接口 ==============

// 初始化 Handler
// deps: 依赖注入，NULL 表示使用默认依赖
flash_error_t flash_handler_init(flash_handler_t *handler,
                                 flash_dependencies_t *deps);

// 去初始化 Handler
flash_error_t flash_handler_deinit(flash_handler_t *handler);

// ============== 注册器模式接口 ==============

// 注册 Flash 设备
// name: 设备名称，用于后续查找
// driver: 驱动实例（需已分配内存）
flash_error_t flash_handler_register(flash_handler_t *handler, const char *name,
                                     flash_driver_t *driver);

// 注销 Flash 设备
flash_error_t flash_handler_unregister(flash_handler_t *handler,
                                       const char *name);

// 查找 Flash 设备
flash_driver_t *flash_handler_find(flash_handler_t *handler, const char *name);

// 获取注册的设备数量
uint8_t flash_handler_get_device_count(flash_handler_t *handler);

// 根据索引获取设备
flash_driver_t *flash_handler_get_device_by_index(flash_handler_t *handler,
                                                  uint8_t index);

// ============== 策略模式接口 ==============

// 设置存储策略
void flash_handler_set_strategy(flash_handler_t *handler,
                                flash_strategy_t *strategy);

// 获取当前策略
flash_strategy_t *flash_handler_get_strategy(flash_handler_t *handler);

// ============== 统一操作接口（代理到驱动或策略） ==============

// 读取数据
// device_name: 设备名称，NULL 表示使用第一个设备
flash_error_t flash_handler_read(flash_handler_t *handler,
                                 const char *device_name, uint32_t addr,
                                 uint8_t *buf, uint32_t len);

// 写入数据
flash_error_t flash_handler_write(flash_handler_t *handler,
                                  const char *device_name, uint32_t addr,
                                  const uint8_t *buf, uint32_t len);

// 擦除扇区
flash_error_t flash_handler_erase_sector(flash_handler_t *handler,
                                         const char *device_name,
                                         uint32_t addr);

// 擦除块
flash_error_t flash_handler_erase_block(flash_handler_t *handler,
                                        const char *device_name, uint32_t addr);

// 擦除区域（自动计算需要擦除的扇区）
flash_error_t flash_handler_erase(flash_handler_t *handler,
                                  const char *device_name, uint32_t addr,
                                  uint32_t len);

// ============== 辅助接口 ==============

// 获取依赖
flash_dependencies_t *flash_handler_get_deps(flash_handler_t *handler);

// 打印所有注册的设备（调试用）
void flash_handler_print_devices(flash_handler_t *handler);

#ifdef __cplusplus
}
#endif

#endif /* INTERFACE_FLASH_HANDLER_H_ */
