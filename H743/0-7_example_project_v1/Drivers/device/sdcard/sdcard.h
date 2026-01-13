#ifndef DRIVERS_DEVICE_SDCARD_SDCARD_H_
#define DRIVERS_DEVICE_SDCARD_SDCARD_H_

#include "../../interface/block_device.h"
#include "sdcard_adapter.h"

typedef struct {
  block_device_t parent;
  sdcard_adapter_t *adapter;
  block_dev_info_t info;
} sdcard_t;

/**
 * @brief Create SD Card device instance
 *
 * @param adapter Initialized adapter instance
 * @return block_device_t* Pointer to block device interface, or NULL on failure
 */
block_device_t *sdcard_create(sdcard_adapter_t *adapter);

/**
 * @brief Destroy SD Card device instance
 *
 * @param dev Pointer to block device interface
 */
void sdcard_destroy(block_device_t *dev);

#endif /* DRIVERS_DEVICE_SDCARD_SDCARD_H_ */
