#ifndef DRIVERS_DEVICE_W25QXX_W25QXX_H_
#define DRIVERS_DEVICE_W25QXX_W25QXX_H_

#include "block_device.h"
#include "w25q_adapter.h"

typedef struct {
  block_device_t parent;
  w25q_adapter_t *adapter;
  block_dev_info_t info;
} w25qxx_t;

/**
 * @brief Create W25Qxx device instance
 * @param adapter Transport adapter (SPI or QSPI)
 */
block_device_t *w25qxx_create(w25q_adapter_t *adapter);
void w25qxx_destroy(block_device_t *dev);

#endif /* DRIVERS_DEVICE_W25QXX_W25QXX_H_ */
