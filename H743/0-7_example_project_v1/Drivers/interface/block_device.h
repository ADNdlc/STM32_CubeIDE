/**
 * @file block_device.h
 * @brief Block Device Interface
 */

#ifndef DRIVERS_INTERFACE_BLOCK_DEVICE_H_
#define DRIVERS_INTERFACE_BLOCK_DEVICE_H_

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Block Device Information
 */
typedef struct {
  uint32_t capacity;    ///< Total capacity in bytes
  uint32_t block_size;  ///< Erase block size in bytes
  uint32_t page_size;   ///< Program page size in bytes
  uint32_t sector_size; ///< Sector size (min erase unit)
  uint8_t erase_value;  ///< Value of erased byte (usually 0xFF)
} block_dev_info_t;

typedef struct block_device_t block_device_t;

/**
 * @brief Block Device Operations
 */
typedef struct {
  /**
   * @brief Initialize the device
   * @return 0 on success, negative on error
   */
  int (*init)(block_device_t *const self);

  /**
   * @brief Deinitialize the device
   * @return 0 on success, negative on error
   */
  int (*deinit)(block_device_t *const self);

  /**
   * @brief Read data from device
   * @param addr Address to read from
   * @param out_buf Buffer to store data
   * @param size Number of bytes to read
   * @return 0 on success, negative on error
   */
  int (*read)(block_device_t *const self, uint32_t addr, uint8_t *out_buf,
              size_t size);

  /**
   * @brief Program data to device
   * @param addr Address to write to
   * @param in_buf Buffer containing data
   * @param size Number of bytes to write
   * @return 0 on success, negative on error
   */
  int (*program)(block_device_t *const self, uint32_t addr,
                 const uint8_t *in_buf, size_t size);

  /**
   * @brief Erase a block/sector
   * @param addr Address of the block/sector
   * @param size Size to erase (must be aligned with erase block size)
   * @return 0 on success, negative on error
   */
  int (*erase)(block_device_t *const self, uint32_t addr, size_t size);

  /**
   * @brief Get device information
   * @param info Pointer to info structure
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
 * @brief Block Device Base Structure
 */
struct block_device_t {
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
