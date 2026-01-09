#ifndef DRIVERS_DEVICE_MT29F4G08_MT29F4G08_ADAPTER_H_
#define DRIVERS_DEVICE_MT29F4G08_MT29F4G08_ADAPTER_H_

#include <stddef.h>
#include <stdint.h>

typedef struct mt29f_adapter_t mt29f_adapter_t;

/**
 * @brief MT29F4G08 Adapter Operations
 */
typedef struct {
  int (*init)(mt29f_adapter_t *self);
  int (*reset)(mt29f_adapter_t *self);
  int (*read_id)(mt29f_adapter_t *self, uint32_t *id);

  /**
   * @brief Read data from a page
   * @param page Address of the page (row address)
   * @param col Column address (within page)
   * @param buf Buffer to store data
   * @param size Number of bytes to read
   */
  int (*read_page)(mt29f_adapter_t *self, uint32_t page, uint32_t col,
                   uint8_t *buf, size_t size);

  /**
   * @brief Program data to a page
   * @param page Address of the page (row address)
   * @param col Column address
   * @param buf Data to write
   * @param size Number of bytes to write
   */
  int (*program_page)(mt29f_adapter_t *self, uint32_t page, uint32_t col,
                      const uint8_t *buf, size_t size);

  /**
   * @brief Erase a block
   * @param block Address of the block (row address aligned to block start)
   */
  int (*erase_block)(mt29f_adapter_t *self, uint32_t block);

  /**
   * @brief Wait for ready status
   * @param timeout_ms Timeout in milliseconds
   */
  int (*wait_busy)(mt29f_adapter_t *self, uint32_t timeout_ms);

} mt29f_adapter_ops_t;

struct mt29f_adapter_t {
  const mt29f_adapter_ops_t *ops;
  void *user_data;
};

#endif /* DRIVERS_DEVICE_MT29F4G08_MT29F4G08_ADAPTER_H_ */
