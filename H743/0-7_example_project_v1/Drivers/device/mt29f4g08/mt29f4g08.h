#ifndef DRIVERS_DEVICE_MT29F4G08_MT29F4G08_H_
#define DRIVERS_DEVICE_MT29F4G08_MT29F4G08_H_

#include "../../interface/block_device.h"
#include "mt29f4g08_adapter.h"

typedef struct {
  block_device_t parent;
  mt29f_adapter_t *adapter;
  block_dev_info_t info;
} mt29f4g08_t;

/**
 * @brief Create MT29F4G08 device instance
 *
 * @param adapter Initialized adapter instance
 * @return block_device_t* Pointer to block device interface, or NULL on failure
 */
block_device_t *mt29f4g08_create(mt29f_adapter_t *adapter);

/**
 * @brief Destroy MT29F4G08 device instance
 *
 * @param dev Pointer to block device interface
 */
void mt29f4g08_destroy(block_device_t *dev);

#endif /* DRIVERS_DEVICE_MT29F4G08_MT29F4G08_H_ */
