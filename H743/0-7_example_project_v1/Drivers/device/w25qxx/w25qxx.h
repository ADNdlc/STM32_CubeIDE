#ifndef DRIVERS_DEVICE_W25QXX_W25QXX_H_
#define DRIVERS_DEVICE_W25QXX_W25QXX_H_

#include "block_device.h"
#include "w25q_adapter.h"

//定义W25Qxx设备ID
#define W25Q256_ID 0x0018

typedef struct {
  block_device_t parent;
  w25q_adapter_t *adapter;
  block_dev_info_t info;
} w25qxx_t;

/**
 * @brief 创建 W25Qxx 设备实例
 * @param adapter 传输适配器 (SPI or QSPI)
 */
block_device_t *w25qxx_create(w25q_adapter_t *adapter);
void w25qxx_destroy(block_device_t *dev);

#endif /* DRIVERS_DEVICE_W25QXX_W25QXX_H_ */
