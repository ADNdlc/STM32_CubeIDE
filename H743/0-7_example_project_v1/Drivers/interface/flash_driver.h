/*
 * flash_driver.h
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  Flash 驱动抽象接口
 *  支持多种 Flash 芯片（W25Q256、W25Q128、W25Q64 等）
 *  采用虚函数表设计，支持多态
 */

#ifndef INTERFACE_FLASH_DRIVER_H_
#define INTERFACE_FLASH_DRIVER_H_

#include <stddef.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

// 前向声明
typedef struct flash_driver_t flash_driver_t;

// Flash 驱动错误码
typedef enum {
  FLASH_OK = 0,        // 操作成功
  FLASH_ERR_PARAM,     // 参数错误
  FLASH_ERR_BUSY,      // 设备忙
  FLASH_ERR_TIMEOUT,   // 超时
  FLASH_ERR_WRITE,     // 写入错误
  FLASH_ERR_ERASE,     // 擦除错误
  FLASH_ERR_READ,      // 读取错误
  FLASH_ERR_PROTECTED, // 写保护
  FLASH_ERR_NOT_INIT,  // 未初始化
  FLASH_ERR_ADDR,      // 地址越界
} flash_error_t;

// Flash 芯片信息
typedef struct {
  uint32_t jedec_id;     // JEDEC ID (Manufacturer + Device ID)
  uint32_t capacity;     // 容量（字节）
  uint32_t sector_size;  // 扇区大小（字节，通常4KB）
  uint32_t block_size;   // 块大小（字节，通常64KB）
  uint32_t page_size;    // 页大小（字节，通常256）
  uint32_t sector_count; // 扇区数量
  uint8_t addr_mode;     // 地址模式：3 = 3字节，4 = 4字节
  const char *name;      // 芯片名称
} flash_info_t;

// Flash 驱动操作接口（虚函数表）
typedef struct {
  // 初始化
  flash_error_t (*init)(flash_driver_t *self);

  // 去初始化
  flash_error_t (*deinit)(flash_driver_t *self);

  // 读取数据
  // addr: 起始地址，buf: 数据缓冲区，len: 读取长度
  flash_error_t (*read)(flash_driver_t *self, uint32_t addr, uint8_t *buf,
                        uint32_t len);

  // 写入数据（页编程，需先擦除）
  // addr: 起始地址，buf: 数据缓冲区，len: 写入长度
  flash_error_t (*write)(flash_driver_t *self, uint32_t addr,
                         const uint8_t *buf, uint32_t len);

  // 擦除扇区（4KB）
  flash_error_t (*erase_sector)(flash_driver_t *self, uint32_t addr);

  // 擦除块（64KB）
  flash_error_t (*erase_block)(flash_driver_t *self, uint32_t addr);

  // 全片擦除
  flash_error_t (*erase_chip)(flash_driver_t *self);

  // 检查是否忙
  int (*is_busy)(flash_driver_t *self);

  // 读取芯片 ID
  uint32_t (*read_id)(flash_driver_t *self);

  // 获取芯片信息
  const flash_info_t *(*get_info)(flash_driver_t *self);

  // 电源管理
  flash_error_t (*power_down)(flash_driver_t *self);
  flash_error_t (*power_up)(flash_driver_t *self);

  // 写保护控制
  flash_error_t (*write_protect)(flash_driver_t *self, int enable);

} flash_driver_ops_t;

// Flash 驱动基类
struct flash_driver_t {
  const flash_driver_ops_t *ops; // 操作接口
  flash_info_t info;             // 芯片信息
  void *priv;                    // 私有数据（子类扩展用）
  char name[16];                 // 设备名称（用于注册）
  uint8_t initialized;           // 初始化标志
};

// ============== 辅助宏定义 ==============

#define FLASH_INIT(drv) ((drv)->ops->init(drv))
#define FLASH_DEINIT(drv) ((drv)->ops->deinit(drv))

#define FLASH_READ(drv, addr, buf, len) ((drv)->ops->read(drv, addr, buf, len))

#define FLASH_WRITE(drv, addr, buf, len)                                       \
  ((drv)->ops->write(drv, addr, buf, len))

#define FLASH_ERASE_SECTOR(drv, addr) ((drv)->ops->erase_sector(drv, addr))

#define FLASH_ERASE_BLOCK(drv, addr) ((drv)->ops->erase_block(drv, addr))

#define FLASH_ERASE_CHIP(drv) ((drv)->ops->erase_chip(drv))

#define FLASH_IS_BUSY(drv) ((drv)->ops->is_busy(drv))
#define FLASH_READ_ID(drv) ((drv)->ops->read_id(drv))
#define FLASH_GET_INFO(drv) ((drv)->ops->get_info(drv))

#define FLASH_POWER_DOWN(drv) ((drv)->ops->power_down(drv))
#define FLASH_POWER_UP(drv) ((drv)->ops->power_up(drv))

#define FLASH_WRITE_PROTECT(drv, en) ((drv)->ops->write_protect(drv, en))

#ifdef __cplusplus
}
#endif

#endif /* INTERFACE_FLASH_DRIVER_H_ */
