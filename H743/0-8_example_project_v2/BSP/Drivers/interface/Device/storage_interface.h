/*
 * block_device.h
 *
 *  Created on: Mar 18, 2026
 *      Author: 12114
 */

#ifndef DRIVERS_INTERFACE_DEVICE_STORAGE_INTERFACE_H_
#define DRIVERS_INTERFACE_DEVICE_STORAGE_INTERFACE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// 前向声明
typedef struct storage_device_t storage_device_t;

/**
 * @brief 存储设备类型
 */
typedef enum {
  STORAGE_TYPE_NOR_FLASH = 0,
  STORAGE_TYPE_NAND_FLASH,
  STORAGE_TYPE_SD_CARD,
  STORAGE_TYPE_RAM_DISK,
  STORAGE_TYPE_EMMC
} storage_type_t;

/**
 * @brief 存储设备状态
 */
typedef enum {
  STORAGE_STATUS_OK = 0,   /* 正常 */
  STORAGE_STATUS_NOT_INIT, /* 未初始化 */
  STORAGE_STATUS_OFFLINE,  /* 离线/未插入 */
  STORAGE_STATUS_BUSY,     /* 忙 */
  STORAGE_STATUS_ERROR     /* 硬件故障 */
} storage_status_t;

typedef enum {
  DEVICE_EVENT_INSERTED,
  DEVICE_EVENT_REMOVED,
  FDEVICE_EVENT_ERROR
} dev_event_t;

typedef void (*storage_callback_t)(storage_device_t *self, dev_event_t event,
                                   void *user_data);

/**
 * @brief 存储设备详细信息
 * 用于文件系统挂载参考
 */
typedef struct {
  storage_type_t type; /* 介质类型 */
  uint64_t total_size; /* 总容量 (Bytes) */
  uint32_t erase_size; /* 最小擦除单位 (Bytes)，NAND 为 Block Size, NOR 为
                          Sector Size */
  uint32_t write_size; /* 最小写入单位 (Bytes)，NAND 为 Page Size, NOR 通常为 1
                          或 256 */
  uint32_t read_size;   /* 最小读取单位 (Bytes)，通常为 1 */
  uint8_t device_id[8]; /* 设备唯一 ID / 制造商 ID */
} storage_info_t;

/**
 * @brief 存储设备操作接口 (Virtual Function Table)
 */
typedef struct {
  /**
   * @brief 存活与使能控制
   */
  int (*init)(storage_device_t *self);   /* 初始化并使能底层总线/外设 */
  int (*deinit)(storage_device_t *self); /* 禁用底层总线/外设 (进入低功耗) */
  storage_status_t (*get_status)(
      storage_device_t *self); /* 存活检测/状态获取 */

  /**
   * @brief 数据操作 (字节偏移地址或块地址取决于文件系统适配)
   */
  int (*read)(storage_device_t *self, uint32_t addr, uint8_t *buf, size_t len);
  int (*write)(storage_device_t *self, uint32_t addr, const uint8_t *data,
               size_t len);
  int (*erase)(storage_device_t *self, uint32_t addr,
               size_t len); /* 擦除操作 */

  /**
   * @brief 扩展控制 (类似于 ioctl)
   * 用于执行 XIP 切换、坏块管理、同步(Sync)等特殊操作
   */
  int (*control)(storage_device_t *self, int cmd, void *arg);

  /**
   * @brief 获取硬件信息
   */
  int (*get_info)(storage_device_t *self, storage_info_t *info);
} storage_ops_t;

/**
 * @brief 存储设备基类结构
 */
struct storage_device_t {
  const storage_ops_t *ops; /* 操作接口 */
  storage_info_t info;      /* 缓存设备信息 */
  storage_callback_t cb;
  void *user_data;
};

/* --- 辅助宏定义，方便上层组件调用 --- */

#define STORAGE_INIT(dev) (dev)->ops->init(dev)
#define STORAGE_DEINIT(dev) (dev)->ops->deinit(dev)
#define STORAGE_CHECK_ALIVE(dev) (dev)->ops->get_status(dev)
#define STORAGE_READ(dev, addr, b, l) (dev)->ops->read(dev, addr, b, l)
#define STORAGE_WRITE(dev, addr, d, l) (dev)->ops->write(dev, addr, d, l)
#define STORAGE_ERASE(dev, addr, l) (dev)->ops->erase(dev, addr, l)
#define STORAGE_GET_INFO(dev, info) (dev)->ops->get_info(dev, info)
#define STORAGE_CONTROL(dev, cmd, arg) (dev)->ops->control(dev, cmd, arg)

#define STORAGE_SET_CB(dev, cb, data)                                          \
  ((dev)->cb = (cb), (dev)->user_data = (data))

#endif /* DRIVERS_INTERFACE_DEVICE_STORAGE_INTERFACE_H_ */
