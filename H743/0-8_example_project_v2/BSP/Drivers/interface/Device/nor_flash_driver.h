/*
 * flash_driver.h
 *
 *  Created on: Feb 19, 2026
 *      Author: Antigravity
 */

#ifndef BSP_DEVICE_DRIVER_INTERFACE_FLASH_DRIVER_H_
#define BSP_DEVICE_DRIVER_INTERFACE_FLASH_DRIVER_H_

#include <stddef.h>
#include <stdint.h>

// 前向声明
typedef struct nor_flash_driver_t nor_flash_driver_t;

/**
 * @brief Flash 信息结构体
 */
typedef struct nor_flash_info_t {
  uint32_t total_size;  /* 总容量 (Bytes) */
  uint32_t sector_size; /* 扇区大小 (Bytes, 擦除最小单位) */
  uint32_t page_size;   /* 页大小 (Bytes, 写入参考单位) */
  uint32_t block_size;  /* 块大小 (Bytes) */
  uint8_t device_id[3]; /* 制造商 + 设备 ID */
} nor_flash_info_t;

/**
 * @brief Flash 工作模式
 */
typedef enum {
  NOR_FLASH_MODE_COMMAND = 0, /* 指令模式 (用于读写擦除) */
  NOR_FLASH_MODE_XIP          /* 内存映射模式 (用于直接运行代码/访问资源) */
} nor_flash_mode_t;

/**
 * @brief Flash 驱动操作接口
 */
typedef struct nor_flash_driver_ops_t {
  // 基本读写擦除
  int (*read)(nor_flash_driver_t *self, uint32_t addr, uint8_t *buf,
              size_t len);
  int (*write)(nor_flash_driver_t *self, uint32_t addr, const uint8_t *data,
               size_t len);
  int (*erase_sector)(nor_flash_driver_t *self, uint32_t addr);
  int (*erase_chip)(nor_flash_driver_t *self);

  // 获取设备信息
  int (*get_info)(nor_flash_driver_t *self, nor_flash_info_t *info);

  // 模式切换
  int (*set_mode)(nor_flash_driver_t *self, nor_flash_mode_t mode);
} nor_flash_driver_ops_t;

/**
 * @brief Flash 驱动基类
 */
typedef struct nor_flash_driver_t {
  const nor_flash_driver_ops_t *ops;
  nor_flash_info_t info; /* Flash 硬件信息 */
} nor_flash_driver_t;

// 辅助宏
#define NOR_FLASH_READ(drv, addr, buf, len)                                    \
  (drv)->ops->read(drv, addr, buf, len)
#define NOR_FLASH_WRITE(drv, addr, data, len)                                  \
  (drv)->ops->write(drv, addr, data, len)
#define NOR_FLASH_ERASE_SECTOR(drv, addr) (drv)->ops->erase_sector(drv, addr)
#define NOR_FLASH_GET_INFO(drv, info) (drv)->ops->get_info(drv, info)
#define NOR_FLASH_SET_MODE(drv, mode) (drv)->ops->set_mode(drv, mode)

#endif /* BSP_DEVICE_DRIVER_INTERFACE_FLASH_DRIVER_H_ */
