/**
 * @file block_device.h
 * @brief Block Device Interface
 */

#ifndef DRIVERS_INTERFACE_BLOCK_DEVICE_H_
#define DRIVERS_INTERFACE_BLOCK_DEVICE_H_

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Block Device 定义
 */
typedef struct
{
  uint32_t capacity;    // 容量(bytes)
  uint32_t block_size;  // 擦除块大小(bytes)
  uint32_t page_size;   // 页大小(bytes)
  uint32_t sector_size; // 扇区大小(block)
  uint8_t erase_value;  // 擦除块填充值(通常 0xFF)
} block_dev_info_t;

typedef struct block_device_t block_device_t;

/**
 * @brief Block Device 操作
 */
typedef struct
{
  /**
   * @brief 初始化
   * @return 0 on success, negative on error
   */
  int (*init)(block_device_t *const self);

  /**
   * @brief 反初始化
   * @return 0 on success, negative on error
   */
  int (*deinit)(block_device_t *const self);

  /**
   * @brief 从设备读取数据
   * @param addr 起始地址
   * @param out_buf 缓冲区
   * @param size 读取大小(bytes)
   * @return 0 on success, negative on error
   */
  int (*read)(block_device_t *const self, uint32_t addr, uint8_t *out_buf,
              size_t size);

  /**
   * @brief 写数据到设备
   * @param addr 起始地址
   * @param in_buf 缓冲区
   * @param size 写入大小(bytes)
   * @return 0 on success, negative on error
   */
  int (*program)(block_device_t *const self, uint32_t addr,
                 const uint8_t *in_buf, size_t size);

  /**
   * @brief 擦除(block/sector)
   * @param addr Address of the block/sector
   * @param size 擦除大小 (必须对齐 block 大小)
   * @return 0 on success, negative on error
   */
  int (*erase)(block_device_t *const self, uint32_t addr, size_t size);

  /**
   * @brief 获取设备信息
   * @param info 输出
   * @return 0 on success, negative on error
   */
  int (*get_info)(block_device_t *const self, block_dev_info_t *info);

  /**
   * @brief Sync/Flush any pending operations
   * @return 0 on success, negative on error
   */
  int (*sync)(block_device_t *const self);

} block_device_ops_t;

/**
 * @brief Block Device 基本结构
 */
struct block_device_t
{
  const block_device_ops_t *ops;
  void *user_data;
};

/* Helper Macros */
#define BLOCK_DEV_INIT(dev) ((dev)->ops->init(dev))
#define BLOCK_DEV_DEINIT(dev) ((dev)->ops->deinit(dev))
#define BLOCK_DEV_READ(dev, a, b, s) ((dev)->ops->read(dev, a, b, s))
#define BLOCK_DEV_PROGRAM(dev, a, b, s) ((dev)->ops->program(dev, a, b, s))
#define BLOCK_DEV_ERASE(dev, a, s) ((dev)->ops->erase(dev, a, s))
#define BLOCK_DEV_GET_INFO(dev, i) ((dev)->ops->get_info(dev, i))
#define BLOCK_DEV_SYNC(dev) ((dev)->ops->sync(dev))

#endif /* DRIVERS_INTERFACE_BLOCK_DEVICE_H_ */
