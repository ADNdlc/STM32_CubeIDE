#ifndef DRIVERS_DEVICE_SDCARD_SDCARD_ADAPTER_H_
#define DRIVERS_DEVICE_SDCARD_SDCARD_ADAPTER_H_

#include <stddef.h>
#include <stdint.h>

/**
 * @brief SD Card Information structure
 */
typedef struct {
  uint32_t block_size;  // Block size in bytes (usually 512)
  uint32_t block_count; // Total number of blocks
  uint32_t capacity_mb; // Capacity in MB
} sdcard_info_t;

typedef struct sdcard_adapter_t sdcard_adapter_t;

/**
 * @brief SD Card Adapter Operations
 */
typedef struct {
  int (*init)(sdcard_adapter_t *self);
  int (*deinit)(sdcard_adapter_t *self);

  /**
   * @brief Read blocks from SD card
   * @param block_addr Logical block address
   * @param buf Buffer to store data
   * @param block_count Number of blocks to read
   */
  int (*read_blocks)(sdcard_adapter_t *self, uint32_t block_addr, uint8_t *buf,
                     uint32_t block_count);

  /**
   * @brief Write blocks to SD card
   * @param block_addr Logical block address
   * @param buf Data to write
   * @param block_count Number of blocks to write
   */
  int (*write_blocks)(sdcard_adapter_t *self, uint32_t block_addr,
                      const uint8_t *buf, uint32_t block_count);

  /**
   * @brief Get SD card information
   */
  int (*get_info)(sdcard_adapter_t *self, sdcard_info_t *info);

  /**
   * @brief Check if card is ready/busy
   */
  int (*is_ready)(sdcard_adapter_t *self);

  /**
   * @brief 检测卡是否物理存在(用于热插拔)
   * @return 1=存在, 0=不存在, -1=需要重新初始化才能确定
   */
  int (*detect)(sdcard_adapter_t *self);

  /**
   * @brief 硬件完全重置(GPIO/外设/时钟)
   * @return 0 on success, negative on error
   */
  int (*reset)(sdcard_adapter_t *self);

} sdcard_adapter_ops_t;

struct sdcard_adapter_t {
  const sdcard_adapter_ops_t *ops;
  void *user_data;
};

#endif /* DRIVERS_DEVICE_SDCARD_SDCARD_ADAPTER_H_ */
